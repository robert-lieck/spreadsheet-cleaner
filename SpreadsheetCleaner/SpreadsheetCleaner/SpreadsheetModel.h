#ifndef SPREADSHEETMODEL_H
#define SPREADSHEETMODEL_H

#include <QAbstractTableModel>
#include <QChar>

#include <vector>

class SpreadsheetModel : public QAbstractTableModel {
    Q_OBJECT
    friend class SpreadsheetCleaner;
    //----members----//
protected:
    QChar input_separator, output_separator, input_quote_char, output_quote_char;
    std::vector<std::vector<QString>> sheet_data;
    std::vector<bool> columns_to_compare, rows_to_delete;
    std::vector<int> duplicate_index;
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
protected:
    void reset(QString file_name);
    void save(QString file_name);
    bool end_of_row(QString line);
    void find_duplicates();
};

#endif // SPREADSHEETMODEL_H
