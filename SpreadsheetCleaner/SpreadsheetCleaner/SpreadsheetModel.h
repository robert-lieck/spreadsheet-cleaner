#ifndef SPREADSHEETMODEL_H
#define SPREADSHEETMODEL_H

#include <QAbstractTableModel>
#include <QChar>
#include <QColor>


#include <vector>
#include <array>

class SpreadsheetModel : public QAbstractTableModel {
    Q_OBJECT
    //----types/classes----//
public:
    enum ROW_BLOCKING_STATE {UNBLOCKED, ALWAYS_DELETE, NEVER_DELETE};
    //----members----//
public:
    const QColor unblocked_unique_color        = QColor(255,255,255); // unblocked and unique         --> keep
    const QColor always_delete_unique_color    = QColor(255, 90, 90); // always deleted BUT unique    --> delete
    const QColor never_delete_unique_color     = QColor(200,255,200); // never deleted and unique     --> keep
    const QColor unblocked_duplicate_color     = QColor(255,200,200); // unblocked and duplicate      --> delete
    const QColor always_delete_duplicate_color = QColor(255,140,140); // always deleted and duplicate --> delete
    const QColor never_delete_duplicate_color  = QColor(100,255,100); // never deleted BUT duplicate  --> keep
    const QColor compare_column_color          = QColor(230,230,255);
    const QColor compare_column_head_color     = QColor(170,170,230);
protected:
    QChar input_separator, output_separator, input_quote_char, output_quote_char;
    std::vector<std::vector<QString>> sheet_data;
    std::vector<QString> line_endings;
    QString line_ending_type;
    std::vector<bool> columns_to_compare, rows_to_delete;
    std::vector<ROW_BLOCKING_STATE> blocked_rows;
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
    void reset(QString file_name);
    void save(QString file_name);
    void set_input_separator(QChar c);
    void set_output_separator(QChar c);
    void set_input_quote_char(QChar c);
    void set_output_quote_char(QChar c);
    void set_columns_compare_status(int idx, bool b);
    bool get_columns_compare_status(int idx);
    void set_row_blocking_status(int row_index, ROW_BLOCKING_STATE value);
    void set_ignore_case(bool b);
    void set_simplify_whitespace(bool b);
signals:
    void info(QString,std::array<std::array<int,4>,3>);
protected:
    void compute_statistics();
    bool equal(QString,QString);
    void find_duplicates();
};

#endif // SPREADSHEETMODEL_H
