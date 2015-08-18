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
    QAction * context_menu_action;
    int toggle_column_index;
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
    void toggle_column();
};

#endif // SPREADSHEETCLEANER_H
