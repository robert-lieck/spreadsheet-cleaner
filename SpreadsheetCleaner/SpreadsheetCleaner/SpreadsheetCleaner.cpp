#include "SpreadsheetCleaner.h"
#include "ui_SpreadsheetCleaner.h"
#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>

SpreadsheetCleaner::SpreadsheetCleaner(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SpreadsheetCleaner),
    spread_sheet_model(this),
    menu(new QMenu(this)),
    context_menu_action(menu->addAction(""))
{
    ui->setupUi(this);
    ui->_wTable->setModel(&spread_sheet_model);
    ui->_wTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->_wTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    connect(context_menu_action, SIGNAL(triggered(bool)), this, SLOT(toggle_column()));
    csv_encoding_changed();
}

SpreadsheetCleaner::~SpreadsheetCleaner() {
    delete ui;
}

void SpreadsheetCleaner::open_file(bool browse) {
    QString file_name = ui->_wOpenFileName->text();
    // check if file exists and is non-empty
    if(file_name=="" || !QFile(file_name).open(QIODevice::ReadOnly)) {
        if(browse) {
            qDebug() << "file" << file_name << "is empty or file does not exist: browse for file";
            browse_open_file();
        } else {
            qDebug() << "cannot open file" << file_name;
        }
    } else {
        qDebug() << "open file" << file_name;
        spread_sheet_model.reset(file_name);
        ui->_wTable->resizeColumnsToContents();
    }
}

void SpreadsheetCleaner::save_file(bool browse) {
    QString file_name = ui->_wSaveFileName->text();
    // check if file exists and is non-empty
    if(file_name=="" || !QFile(file_name).open(QIODevice::WriteOnly | QIODevice::Text)) {
        if(browse) {
            qDebug() << "file" << file_name << "is empty or file does not exist: browse for file";
            browse_save_file();
        } else {
            qDebug() << "cannot save to file" << file_name;
        }
    } else {
        qDebug() << "save to file" << file_name;
        spread_sheet_model.save(file_name);
    }
}

void SpreadsheetCleaner::browse_open_file() {
    QString base_path = "";
    QString old_file_name = ui->_wOpenFileName->text();
    if(old_file_name!="") {
        base_path = QFileInfo(old_file_name).absolutePath();
    }
    QString file_name = QFileDialog::getOpenFileName(this, tr("Open file:"), base_path, tr("CSV (*.csv);; Text (*.txt);; Other (*)"));
    if(file_name!="") {
        ui->_wOpenFileName->setText(file_name);
        open_file(false);
    } else {
        qDebug() << "no file selected";
    }
}

void SpreadsheetCleaner::browse_save_file() {
    QString base_path = "";
    QString old_file_name = ui->_wSaveFileName->text();
    if(old_file_name!="") {
        base_path = QFileInfo(old_file_name).absolutePath();
    }
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save file:"), base_path, tr("CSV (*.csv);; Text (*.txt);; Other (*)"));
    if(file_name!="") {
        ui->_wSaveFileName->setText(file_name);
        save_file(false);
    } else {
        qDebug() << "no file selected";
    }
}

void SpreadsheetCleaner::csv_encoding_changed() {
    QChar input_separator;
    if(ui->_wFieldSeparatorComma->isChecked()) input_separator = ',';
    if(ui->_wFieldSeparatorSemicolon->isChecked()) input_separator = ';';
    if(ui->_wFieldSeparatorOther->isChecked()) {
        QString value = ui->_wFieldSeparatorOtherValue->text();
        if(value.size()>0) {
            input_separator = value[0];
            ui->_wFieldSeparatorOtherValue->setText(input_separator);
        } else {
            input_separator = QChar::Null;
        }
    }
    spread_sheet_model.input_separator = input_separator;
    if(!ui->_wFieldSeparatorOutputGroup->isChecked()) {
        spread_sheet_model.output_separator = input_separator;
    } else {
        QChar output_separator;
        if(ui->_wFieldSeparatorOutputComma->isChecked()) output_separator = ',';
        if(ui->_wFieldSeparatorOutputSemicolon->isChecked()) output_separator = ';';
        if(ui->_wFieldSeparatorOutputOther->isChecked()) {
            QString value = ui->_wFieldSeparatorOutputOtherValue->text();
            if(value.size()>0) {
                output_separator = value[0];
                ui->_wFieldSeparatorOutputOtherValue->setText(output_separator);
            } else {
                output_separator = QChar::Null;
            }
        }
        spread_sheet_model.output_separator = output_separator;
    }
    QChar input_quote_char;
    if(ui->_wFieldQuotingDoubleQuotes->isChecked()) input_quote_char = '"';
    if(ui->_wFieldQuotingSingleQuotes->isChecked()) input_quote_char = '\'';
    if(ui->_wFieldQuotingOther->isChecked()) {
        QString value= ui->_wFieldQuotingOtherValue->text();
        if(value.size()>0) {
            input_quote_char = value[0];
            ui->_wFieldQuotingOtherValue->setText(input_quote_char);
        } else {
            input_quote_char = QChar::Null;
        }
    }
    spread_sheet_model.input_quote_char = input_quote_char;
    if(!ui->_wFieldQuotingOutputGroup->isChecked()) {
        spread_sheet_model.output_quote_char = input_quote_char;
    } else {
        QChar output_quote_char;
        if(ui->_wFieldQuotingOutputDoubleQuotes->isChecked()) output_quote_char = '"';
        if(ui->_wFieldQuotingOutputSingleQuotes->isChecked()) output_quote_char = '\'';
        if(ui->_wFieldQuotingOutputOther->isChecked()) {
            QString value = ui->_wFieldQuotingOutputOtherValue->text();
            if(value.size()>0) {
                output_quote_char = value[0];
                ui->_wFieldQuotingOutputOtherValue->setText(output_quote_char);
            } else {
                output_quote_char = QChar::Null;
            }
        }
        spread_sheet_model.output_quote_char = output_quote_char;
    }
}

void SpreadsheetCleaner::make_field_separator_other() {
    ui->_wFieldSeparatorOther->setChecked(true);
}

void SpreadsheetCleaner::make_field_separator_output_other() {
    ui->_wFieldSeparatorOutputOther->setChecked(true);
}

void SpreadsheetCleaner::make_field_quoting_other() {
    ui->_wFieldQuotingOther->setChecked(true);
}

void SpreadsheetCleaner::make_field_quoting_output_other() {
    ui->_wFieldQuotingOutputOther->setChecked(true);
}

void SpreadsheetCleaner::contextMenu(QPoint pos) {
    QModelIndex index = ui->_wTable->indexAt(pos);
    if(index.isValid()) {
        if(spread_sheet_model.columns_to_compare[index.column()]) {
            menu->actions()[0]->setText("ignore column");
        } else {
            menu->actions()[0]->setText("compare column");
        }
        toggle_column_index = index.column();
        if(spread_sheet_model.duplicate_index[index.row()]>=0) {
            menu->setTitle(QString("Is duplicate of row %1").arg(spread_sheet_model.duplicate_index[index.row()]));
        } else {
            menu->setTitle("");
        }
        menu->popup(ui->_wTable->viewport()->mapToGlobal(pos));
    }
}

void SpreadsheetCleaner::toggle_column() {
    spread_sheet_model.toggle_column(toggle_column_index);
}
