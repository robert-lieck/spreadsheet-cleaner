#ifndef SPREADSHEETCLEANER_H
#define SPREADSHEETCLEANER_H

#include <QMainWindow>

#include "SpreadsheetModel.h"

namespace Ui {
class SpreadsheetCleaner;
}

class SpreadsheetCleaner : public QMainWindow
{
    Q_OBJECT

public:
    explicit SpreadsheetCleaner(QWidget *parent = 0);
    ~SpreadsheetCleaner();
private:
    Ui::SpreadsheetCleaner *ui;
    SpreadsheetModel spread_sheet_model;
    QMenu * menu;
    QAction * compare_column_action;
    QAction * ignore_column_action;
    QAction * always_delete_row_action;
    QAction  * never_delete_row_action;
    QAction * unblock_row_action;
    int context_menu_row_index, context_menu_column_index;
private slots:
    void open_file(bool browse = true);
    void save_file(bool browse = true);
    void browse_open_file();
    void browse_save_file();
    void csv_encoding_changed();
    void make_field_separator_other();
    void make_field_separator_output_other();
    void make_field_quoting_other();
    void make_field_quoting_output_other();
    void contextMenu(QPoint);
    void set_compare_column_action();
    void set_ignore_column_action();
    void set_always_delete_row_action();
    void set_never_delete_row_action();
    void set_unblock_row_action();
    void ignore_case_toggled(bool);
    void simplify_whitespace_toggled(bool);
    void info(QString info_text, std::array<std::array<int,4>,3> stats);
};

#endif // SPREADSHEETCLEANER_H
