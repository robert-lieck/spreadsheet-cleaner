#include "SpreadsheetModel.h"
#include <QDebug>
#include <QFont>
#include <QBrush>
#include <QFile>

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
        if(rows_to_delete[row]) {
            return QBrush(QColor(255,230,230));
        } else if(columns_to_compare[col]) {
            return QBrush(QColor(230,230,255));
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
    }
    return QVariant();
}

bool SpreadsheetModel::setData(const QModelIndex & index, const QVariant & value, int role) {
    switch(role) {
    case Qt::EditRole:
        sheet_data[index.row()][index.column()] = value.toString();
        break;
    case Qt::CheckStateRole:
        break;
    }
    return true;
}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex & /*index*/) const {
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

void SpreadsheetModel::toggle_column(int column_index) {
    columns_to_compare[column_index] = !columns_to_compare[column_index];
    find_duplicates();
    emit dataChanged(createIndex(0,column_index),createIndex(sheet_data.size()-1,column_index));
}

void SpreadsheetModel::reset(QString file_name) {
    beginResetModel();
    QFile file(file_name);
    if(!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << file.errorString();
    } else {
        // parse the csv file
        sheet_data.clear();
        int max_cols = 0;
        while(!file.atEnd()) {
//            qDebug() << "new row";
            sheet_data.push_back(std::vector<QString>());
            std::vector<QString> & row = sheet_data.back();
            QString line = file.readLine();
#if 1
            enum STATE {ROW_START,         // 0
                        ON_SEPARATOR,      // 1
                        ON_QUOTE_BEGIN,    // 2
                        ON_QUOTE_END,      // 3
                        IN_UNQUOTED_FIELD, // 4
                        IN_QUOTED_FIELD,   // 5
                        ROW_END};
            STATE state = ROW_START;
            QString current_field = "";
//            qDebug() << "input separator:" << input_separator;
//            qDebug() << "input quoting:" << input_quote_char;
            while(state!=ROW_END) {
//                if(line.size()>0) qDebug() << state << line[0] << QString("(%1)").arg(line.size());
                switch(state) {
                case ROW_START:
                    if(end_of_row(line)) state = ROW_END;
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
                    if(end_of_row(line)) {
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
                    if(end_of_row(line)) {
                        qDebug() << "Error: premature line end after opening quote";
                        row.push_back(current_field);
                        state = ROW_END;
                    } else if(line[0]==input_quote_char) state = ON_QUOTE_END;
                    else state = IN_QUOTED_FIELD;
                    break;
                case ON_QUOTE_END:
                    line.remove(0,1);
                    if(end_of_row(line)) {
                        row.push_back(current_field);
                        state = ROW_END;
                    } else if(line[0]==input_quote_char) {
                        // MS Excel uses two quote characters in sequence inside
                        // a quoted field to indicate the quote character as content
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
                    if(end_of_row(line)) {
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
                        if(end_of_row(line)) {
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
#else
            for(QString cell : line.split(',')) {
                row.push_back(cell);
            }
#endif
//            qDebug() << "Row:";
//            for(auto field : row) qDebug() << field;
            max_cols = std::max<int>(max_cols,row.size());
        }
        for(auto & row : sheet_data) {
            row.resize(max_cols,"");
        }
        columns_to_compare.assign(max_cols,false);
        rows_to_delete.assign(sheet_data.size(),false);
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
    QFile file(file_name);
    if(!file.open(QFile::WriteOnly | QFile::Text)) {
        qDebug() << file.errorString();
    } else {
        QTextStream out(&file);
        for(std::vector<QString> & row : sheet_data) {
            QString line = "";
            for(QString field : row) {
                if(field.contains(output_quote_char)) {
                    field.replace(output_quote_char,QString("%1%1").arg(output_quote_char));
                    field = output_quote_char + field + output_quote_char;
                }
                if(line!="") {
                    line += output_separator;
                }
                line += field;
            }
            out << line << "\n";
        }
    }
}

bool SpreadsheetModel::end_of_row(QString line) {
    return line.size()==0 || line[0]=='\n' || line[0]=='\xa';
}

void SpreadsheetModel::find_duplicates() {
    for(int row_idx=0; row_idx<(int)sheet_data.size(); ++row_idx) {
//        qDebug() << "checking row" << row_idx;
        bool found_duplicate = false;
        for(int other_row_idx=row_idx+1; other_row_idx<(int)sheet_data.size(); ++other_row_idx) {
//            qDebug() << "comparing to row" << other_row_idx;
            bool equal = true;
            for(int col_idx=0; col_idx<(int)sheet_data[row_idx].size(); ++col_idx) {
                if(columns_to_compare[col_idx]) {
//                    qDebug() << "check column" << col_idx;
                    if(sheet_data[row_idx][col_idx]!=sheet_data[other_row_idx][col_idx]) {
                        equal = false;
//                        qDebug() << "NOT equal";
                        break;
                    } else {
//                        qDebug() << "equal";
                    }
                }
            }
            if(equal) {
//                qDebug() << row_idx << "is duplicate of" << other_row_idx;
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
}
