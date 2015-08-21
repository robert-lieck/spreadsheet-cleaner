#include "SpreadsheetModel.h"
#include <QDebug>
#include <QFont>
#include <QBrush>
#include <QFile>
#include <QRegularExpression>

#include <algorithm>
#include <set>

SpreadsheetModel::SpreadsheetModel(QObject *parent):
    QAbstractTableModel(parent)
{}

int SpreadsheetModel::rowCount(const QModelIndex & /*parent*/) const {
   return sheet_data.size();
}

int SpreadsheetModel::columnCount(const QModelIndex & /*parent*/) const {
    if(sheet_data.size()>0) {
        return sheet_data[0].size();
    } else {
        return 0;
    }
}

QVariant SpreadsheetModel::data(const QModelIndex &index, int role) const {
    int row = index.row();
    int col = index.column();
    switch(role) {
    case Qt::DisplayRole:
        return sheet_data[row][col];
        break;
    case Qt::FontRole:
        break;
    case Qt::BackgroundRole:
        if(blocked_rows[row]==UNBLOCKED) {
            if(rows_to_delete[row]) {
                return QBrush(unblocked_duplicate_color);
            } else {
                if(columns_to_compare[col]) {
                    return QBrush(compare_column_color);
                } else {
                    return QBrush(unblocked_unique_color);
                }
            }
        } else if(blocked_rows[row]==ALWAYS_DELETE) {
            if(rows_to_delete[row]) {
                return QBrush(always_delete_duplicate_color);
            } else {
                return QBrush(always_delete_unique_color);
            }
        } else if(blocked_rows[row]==NEVER_DELETE) {
            if(rows_to_delete[row]) {
                return QBrush(never_delete_duplicate_color);
            } else {
                return QBrush(never_delete_unique_color);
            }
        }
        break;
    case Qt::EditRole:
        return sheet_data[row][col];
        break;
    case Qt::ToolTipRole:
        if(rows_to_delete[row]) {
            return QString("duplicate of %1").arg(duplicate_index[row]);
        }
        break;
    case Qt::TextAlignmentRole:
        break;
    case Qt::CheckStateRole:
        break;
    }
    return QVariant();
}

QVariant SpreadsheetModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role==Qt::DisplayRole) {
        if(orientation == Qt::Horizontal) {
            QString label = "";
            int rest = section;
            while(true) {
                int idx = rest%letters.size();
                label = letters[idx] + label;
                if(idx==rest) break;
                rest = rest / letters.size() - 1;
            }
            return label;
        } else if(orientation == Qt::Vertical) {
            return QString::number(section);
        }
    } else if(role==Qt::BackgroundRole) {
        if(orientation == Qt::Horizontal) {
            if(columns_to_compare[section]) {
                return QBrush(compare_column_head_color);
            }
        }
    }
    return QVariant();
}

bool SpreadsheetModel::setData(const QModelIndex & index, const QVariant & value, int role) {
    switch(role) {
    case Qt::EditRole:
        sheet_data[index.row()][index.column()] = value.toString();
        find_duplicates();
        break;
    case Qt::CheckStateRole:
        break;
    }
    return true;
}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex & /*index*/) const {
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

bool SpreadsheetModel::equal(QString s1, QString s2) {
    if(ignore_case) {
        s1 = s1.toLower();
        s2 = s2.toLower();
    }
    if(simplify_whitespace) {
        s1 = s1.simplified();
        s2 = s2.simplified();
    }
    return s1==s2;
}

void SpreadsheetModel::reset(QString file_name) {
    beginResetModel();
    QFile file(file_name);
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString();
    } else {
        // read whole file and clear old data
        QString file_content = file.readAll();
        // split file content into lines removing line breaks
        static QRegularExpression newline("[\r\n]+",QRegularExpression::OptimizeOnFirstUsageOption);
        QStringList lines = file_content.split(newline);
        sheet_data.assign(lines.size(),std::vector<QString>());
        // get the line breaks
        {
            QRegularExpressionMatchIterator matched_line_endings = newline.globalMatch(file_content);
            line_endings.clear();
            while(matched_line_endings.hasNext()) line_endings.push_back(matched_line_endings.next().captured());
            line_endings.resize(lines.size(),"");
            // find prevailing type
            {
                int ms = 0;    // CR+LF = "\r\n"
                int unix = 0;  // LF    = "\n"
                int mac = 0;   // CR    = "\r"
                int empty = 0;
                int other = 0;
                std::set<QString> other_set;
                for(QString ending : line_endings) {
                    if(ending=="\r\n") ++ms;
                    else if(ending=="\n") ++unix;
                    else if(ending=="\r") ++mac;
                    else if(ending=="") ++empty;
                    else {
                        if(other_set.find(ending)==other_set.end()) {
                            other_set.insert(ending);
                            qDebug() << "Unknown line ending:" << ending;
                        }
                        ++other;
                    }
                }
                if(empty>1) qDebug() << QString("Error (this should be impossible): found more than one line (%1) ending with an empty line break").arg(empty);
                std::vector<std::pair<int,QString>> sorted({{ms,"MS"},{unix,"Unix"},{mac,"Mac"},{other,"other"}});
                std::sort(sorted.begin(),sorted.end());
                line_ending_type = sorted.back().second;
                qDebug() << QString("prevailing line ending type: %1 (%2 of %3)").arg(line_ending_type).arg(sorted.back().first).arg(line_endings.size()-empty);
            }
        }
        // parse each line separately
        int max_cols = 0;
        for(int line_idx=0; line_idx<lines.size(); ++line_idx) {
//            qDebug() << "new row";
            std::vector<QString> & row = sheet_data[line_idx];
            QString line = lines[line_idx];
            QString current_field = "";
            // state of the parser
            enum STATE {ROW_START,         // 0
                        ON_SEPARATOR,      // 1
                        ON_QUOTE_BEGIN,    // 2
                        ON_QUOTE_END,      // 3
                        IN_UNQUOTED_FIELD, // 4
                        IN_QUOTED_FIELD,   // 5
                        ROW_END};
            STATE state = ROW_START;
            while(state!=ROW_END) {
//                if(line.size()>0) qDebug() << state << line[0] << QString("(%1)").arg(line.size());
                switch(state) {
                case ROW_START:
                    if(line.size()==0) state = ROW_END;
                    else if(line[0]==input_quote_char) state = ON_QUOTE_BEGIN;
                    else if(line[0]==input_separator) {
                        row.push_back("");
                        state = ON_SEPARATOR;
                    } else {
                        state = IN_UNQUOTED_FIELD;
                    }
                    break;
                case ON_SEPARATOR:
                    current_field = "";
                    line.remove(0,1);
                    if(line.size()==0) {
                        row.push_back("");
                        state = ROW_END;
                    } else if(line[0]==input_quote_char) state = ON_QUOTE_BEGIN;
                    else if(line[0]==input_separator) {
                        row.push_back("");
                        state = ON_SEPARATOR;
                    } else {
                        state = IN_UNQUOTED_FIELD;
                    }
                    break;
                case ON_QUOTE_BEGIN:
                    line.remove(0,1);
                    if(line.size()==0) {
                        qDebug() << "Error: premature line end after opening quote";
                        row.push_back(current_field);
                        state = ROW_END;
                    } else if(line[0]==input_quote_char) state = ON_QUOTE_END;
                    else state = IN_QUOTED_FIELD;
                    break;
                case ON_QUOTE_END:
                    line.remove(0,1);
                    if(line.size()==0) {
                        row.push_back(current_field);
                        state = ROW_END;
                    } else if(line[0]==input_quote_char) {
                        // MS Excel uses two subsequent quote characters inside
                        // a quoted field to indicate the quote character itself
                        current_field += input_quote_char;
                        line.remove(0,1);
                        state = IN_QUOTED_FIELD;
                    } else if(line[0]==input_separator) {
                        row.push_back(current_field);
                        current_field = "";
                        state = ON_SEPARATOR;
                    } else {
                        qDebug() << "Error: unexpected character" << line[0] << "after closing quote";
                        state = ROW_END;
                    }
                    break;
                case IN_UNQUOTED_FIELD:
                    if(line.size()==0) {
                        row.push_back(current_field);
                        state = ROW_END;
                    } else if(line[0]==input_separator) {
                        row.push_back(current_field);
                        current_field = "";
                        state = ON_SEPARATOR;
                    } else {
                        current_field += line[0];
                        line.remove(0,1);
                    }
                    break;
                case IN_QUOTED_FIELD:
                    if(line[0]==input_quote_char) {
                        state = ON_QUOTE_END;
                    } else {
                        current_field += line[0];
                        line.remove(0,1);
                        if(line.size()==0) {
                            qDebug() << "Error: premature line end within quoted field";
                            state = ROW_END;
                        } else {
                            // nothing to do, just keep on reading the quoted fiel
                        }
                    }
                    break;
                case ROW_END:
                    // nothing more to do
                    break;
                }
            }
//            qDebug() << "Row:";
//            for(auto field : row) qDebug() << field;
            max_cols = std::max<int>(max_cols,row.size());
        }
        for(auto & row : sheet_data) {
            row.resize(max_cols,"");
        }
        columns_to_compare.assign(max_cols,false);
        rows_to_delete.assign(sheet_data.size(),false);
        blocked_rows.assign(sheet_data.size(),UNBLOCKED);
        duplicate_index.assign(sheet_data.size(),-1);
        find_duplicates();
    }
//    for(auto & row : sheet_data) {
//        qDebug() << "Row:";
//        for(auto & field : row) {
//            qDebug() << field;
//        }
//    }
    endResetModel();
}

void SpreadsheetModel::save(QString file_name) {
    bool convert_line_endings = false;
    QFile file(file_name);
    bool success;
    if(convert_line_endings) success = file.open(QIODevice::WriteOnly | QIODevice::Text);
    else success = file.open(QIODevice::WriteOnly);
    if(!success) {
        qDebug() << file.errorString();
    } else {
        QTextStream out(&file);
        for(int line_idx=0; line_idx<(int)sheet_data.size(); ++line_idx) {
            if(blocked_rows[line_idx]==NEVER_DELETE || (blocked_rows[line_idx]==UNBLOCKED && !rows_to_delete[line_idx])) {
                QString line = "";
                for(QString field : sheet_data[line_idx]) {
                    if(field.contains(output_quote_char) || field.contains(output_separator)) {
                        field.replace(output_quote_char,QString("%1%1").arg(output_quote_char));
                        field = output_quote_char + field + output_quote_char;
                    }
                    if(line!="") {
                        line += output_separator;
                    }
                    line += field;
                }
                if(convert_line_endings) {
                    out << line;
                    // don't put a new-line at the end of the file
                    if(line_idx<(int)sheet_data.size()-1) out << "\n";
                } else {
                    out << line << line_endings[line_idx];
                }
            }
        }
    }
}

void SpreadsheetModel::set_input_separator(QChar c) {input_separator = c;}

void SpreadsheetModel::set_output_separator(QChar c) {output_separator = c;}

void SpreadsheetModel::set_input_quote_char(QChar c) {input_quote_char = c;}

void SpreadsheetModel::set_output_quote_char(QChar c) {output_quote_char = c;}

void SpreadsheetModel::set_columns_compare_status(int column_index, bool b) {
    columns_to_compare[column_index] = b;
    emit headerDataChanged(Qt::Horizontal,column_index,column_index);
    find_duplicates();
}

bool SpreadsheetModel::get_columns_compare_status(int idx) {return columns_to_compare[idx];}

void SpreadsheetModel::set_row_blocking_status(int row_index, SpreadsheetModel::ROW_BLOCKING_STATE value) {
    blocked_rows[row_index] = value;
    emit dataChanged(createIndex(0,row_index),createIndex(sheet_data.size()-1,row_index));
    compute_statistics();
}

void SpreadsheetModel::set_ignore_case(bool b) {
    ignore_case = b;
    find_duplicates();
}

void SpreadsheetModel::set_simplify_whitespace(bool b) {
    simplify_whitespace = b;
    find_duplicates();
}

void SpreadsheetModel::compute_statistics() {
    std::array<std::array<int,4>,3> stats({std::array<int,4>({0,0,0,0}),
                                           std::array<int,4>({0,0,0,0}),
                                           std::array<int,4>({0,0,0,0})});
    for(int row_idx=0; row_idx<(int)sheet_data.size(); ++row_idx) {
        if(rows_to_delete[row_idx]) {
            switch(blocked_rows[row_idx]) {
            case UNBLOCKED:
                ++stats[0][0];
                ++stats[2][0];
                break;
            case ALWAYS_DELETE:
                ++stats[0][1];
                ++stats[2][1];
                break;
            case NEVER_DELETE:
                ++stats[0][2];
                ++stats[2][2];
                break;
            }
            ++stats[0][3];
        } else {
            switch(blocked_rows[row_idx]) {
            case UNBLOCKED:
                ++stats[1][0];
                ++stats[2][0];
                break;
            case ALWAYS_DELETE:
                ++stats[1][1];
                ++stats[2][1];
                break;
            case NEVER_DELETE:
                ++stats[1][2];
                ++stats[2][2];
                break;
            }
            ++stats[1][3];
        }
        ++stats[2][3];
    }
    emit info(QString("%1 rows, %2 deleted").arg(stats[2][3]).arg(stats[0][0]+stats[0][1]+stats[1][1]), stats);
}

void SpreadsheetModel::find_duplicates() {
    qDebug() << "find duplicates";
    for(int row_idx=0; row_idx<(int)sheet_data.size(); ++row_idx) {
        bool found_duplicate = false;
        for(int other_row_idx=row_idx+1; other_row_idx<(int)sheet_data.size(); ++other_row_idx) {
            bool fields_are_equal = true;
            for(int col_idx=0; col_idx<(int)sheet_data[row_idx].size(); ++col_idx) {
                if(columns_to_compare[col_idx]) {
                    if(!equal(sheet_data[row_idx][col_idx],sheet_data[other_row_idx][col_idx])) {
                        fields_are_equal = false;
                        break;
                    }
                }
            }
            if(fields_are_equal) {
                found_duplicate = true;
                duplicate_index[row_idx] = other_row_idx;
                break;
            }
        }
        if(found_duplicate!=rows_to_delete[row_idx]) {
                rows_to_delete[row_idx] = found_duplicate;
                if(!found_duplicate) duplicate_index[row_idx] = -1;
                emit dataChanged(createIndex(row_idx,0),createIndex(row_idx,sheet_data[row_idx].size()-1));
        }
    }
    compute_statistics();
}
