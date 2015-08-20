#ifndef SPREADSHEETMODEL_H
#define SPREADSHEETMODEL_H

#include <QAbstractTableModel>
#include <QChar>

#include <vector>

class SpreadsheetModel : public QAbstractTableModel {
    Q_OBJECT
    //----members----//
protected:
    QChar input_separator, output_separator, input_quote_char, output_quote_char;
    std::vector<std::vector<QString>> sheet_data;
    std::vector<bool> columns_to_compare, rows_to_delete;
    std::vector<int> duplicate_index;
    bool ignore_case, simplify_whitespace;
    QString letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    //----methods----//
public:
    SpreadsheetModel(QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &) const override;
    void toggle_column(int column_index);
    void reset(QString file_name);
    void save(QString file_name);
    void set_input_separator(QChar c);
    void set_output_separator(QChar c);
    void set_input_quote_char(QChar c);
    void set_output_quote_char(QChar c);
    void set_columns_to_compare(int idx, bool b);
    bool get_columns_to_compare(int idx);
    void set_ignore_case(bool b);
    void set_simplify_whitespace(bool b);
signals:
    void info(QString);
protected:
    bool equal(QString,QString);
    bool end_of_row(QString line);
    void find_duplicates();
};

#endif // SPREADSHEETMODEL_H
