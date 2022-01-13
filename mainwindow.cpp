#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QDate>
#include <QTime>
#include <QKeyEvent>

using namespace std;

static QString StreamS;
static QSerialPort arduino_serial;
static QString arduino_portName;
static bool arduino_available;
static QString QSbuf;

static QDate curr_d;



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
   ui->setupUi(this);
   // Таймер, его инициал. и настройки
   tmr = new QTimer(); // Создаем объект класса QTimer и передаем адрес переменной
   tmr->setInterval(1000); // Задаем интервал таймера
   connect(tmr, SIGNAL(timeout()), this, SLOT(updateTime())); // Подключаем сигнал таймера к нашему слоту
   tmr->start(); // Запускаем таймер

   // read avaible comports
   // обнаружение возможных портов и вывод в виде списка в comboBox
       foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
       {
               ui->comboBox->addItem(info.portName());
               ui->label_4->setText("Не подключено");
       }

       if (ui->comboBox->currentText() == "" ) //если ничего не подключено, и ничего не внесено в список
       {                                       //combobox, то делаем кнопки подключения и отключения
           ui->pushButton_5->setDisabled(true);// не активными.
           ui->pushButton_6->setDisabled(true);
           ui->label_4->setText("Нет возможных портов для подключения");
       }

       arduino_available = false; // Флаг открытия порта

       // Main menu
        // Меню Файл -> Выход
         QAction *quit = new QAction("&Выход",ui->menu);
           connect(quit, SIGNAL(triggered()), this, SLOT(showMessage()));
           ui->menu->addAction(quit);
           connect(quit, &QAction::triggered, qApp, QApplication::quit);
}

MainWindow::~MainWindow()
{
    if (arduino_available != false)
    {
        arduino_serial.close();
    }
    delete ui;
}

// Timer
void MainWindow::updateTime()
{
    QString curr_timer = QTime::currentTime().toString();

    // Получаем текущие дату и время
    // возвращаем текущую дату
    // возвращаем текущее время

    QString Str_datetime = "";
    Str_datetime.append(curr_d.currentDate().toString(Qt::DefaultLocaleShortDate) );
    Str_datetime.append("  LT: ");
    Str_datetime.append(curr_timer);
    ui->label->setText(Str_datetime);
}

// Обработчик клавиатурных событий
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) // биндинг Esc на выход из пргораммы
    {
          qApp->quit();
    }
    if (event->key() == Qt::Key_W) // бинд W и управл. осью "Dec up"
    {
          on_pushButton_3_pressed();
    }
    if (event->key() == Qt::Key_X) // бинд X и управл. осью "Dec down"
    {
          on_pushButton_4_pressed();
    }
    if (event->key() == Qt::Key_A) // бинд A и управл. осью "Ra left"
    {
          on_pushButton_pressed();
    }
    if (event->key() == Qt::Key_D) // бинд D и управл. осью "Ra right"
    {
          on_pushButton_2_pressed();
    }
    if (event->key() == Qt::Key_S) // бинд S и управл. "STOP"
    {
          on_pushButton_9_clicked();
    }
    if (event->key() == Qt::Key_PageUp) // бинд PageUp и управл. фокусером +
    {
          on_pushButton_8_clicked();
    }
    if (event->key() == Qt::Key_PageDown) // бинд PageDown и управл. фокусером -
    {
          on_pushButton_7_clicked();
    }
    //////// Скорости осей ...................................................
    if (event->key() == Qt::Key_Q) // бинд Q и управл. скорости оси Dec +
    {
         int num;
         num = ui->dial_2->value();
         ui->dial_2->setValue(num+1);
    }
    if (event->key() == Qt::Key_Z) // бинд Z и управл. скорости оси Dec -
    {
         int num;
         num = ui->dial_2->value();
         ui->dial_2->setValue(num-1);
    }
    if (event->key() == Qt::Key_E) // бинд E и управл. скорости оси Ra +
    {
         int num;
         num = ui->dial_3->value();
         ui->dial_3->setValue(num+1);
    }
    if (event->key() == Qt::Key_C) // бинд C и управл. скорости оси Ra -
    {
         int num;
         num = ui->dial_3->value();
         ui->dial_3->setValue(num-1);
    }
    if (event->key() == Qt::Key_Plus) // бинд + и управл. скорости оси фокусера +
    {
         int num;
         num = ui->dial->value();
         ui->dial->setValue(num+1);
    }
    if (event->key() == Qt::Key_Minus) // бинд - и управл. скорости оси фокусера -
    {
         int num;
         num = ui->dial->value();
         ui->dial->setValue(num-1);
    }
}

void MainWindow::on_pushButton_pressed()
{
    StreamS = "<"; // Управляющий символ определяющий ось и направление вращения
    StreamS.append(QString::number(ui->spinBox2->value())); //скорость этого вращения
    // добавляется к управляющему символу, полный сигнал состоит из двух символов
    ui->textEdit->append(StreamS); // вывод (проверка) команды

    // передача команды в порт
    if (arduino_serial.isWritable()) {
           arduino_serial.write(StreamS.toUtf8()); // непосредственно отправка в порт с предварит. конвертацией
            arduino_serial.waitForBytesWritten();
            while (arduino_serial.waitForReadyRead(1))
            {
                 arduino_serial.readAll();
            }

            QByteArray data;  // специальный тип QT для хранения последовательности байтов
                while (arduino_serial.waitForReadyRead(3)) {
                    // вместо холостого чтения накапливаем результат в переменную data
                    data.append(arduino_serial.readAll());
                }
            ui->textEdit_2->append(data);
       }     // в случае проблем
       else {
           QMessageBox::information(this, "Serial Port Error", "Couldn't write to Serial port.");
       }
}

void MainWindow::on_pushButton_2_pressed()
{
    StreamS = ">"; // Управляющий символ определяющий ось и направление вращения
    StreamS.append(QString::number(ui->spinBox2->value())); //скорость этого вращения
    // добавляется к управляющему символу, полный сигнал состоит из двух символов
    ui->textEdit->append(StreamS); // вывод (проверка) команды

    // передача команды в порт
    if (arduino_serial.isWritable()) {
           arduino_serial.write(StreamS.toUtf8()); // непосредственно отправка в порт с предварит. конвертацией
            arduino_serial.waitForBytesWritten();
            while (arduino_serial.waitForReadyRead(1))
            {
                 arduino_serial.readAll();
            }

            QByteArray data;  // специальный тип QT для хранения последовательности байтов
                while (arduino_serial.waitForReadyRead(3)) {
                    // вместо холостого чтения накапливаем результат в переменную data
                    data.append(arduino_serial.readAll());
                }
            ui->textEdit_2->append(data);
       }     // в случае проблем
       else {
           QMessageBox::information(this, "Serial Port Error", "Couldn't write to Serial port.");
       }
}

void MainWindow::on_pushButton_3_pressed()
{
    StreamS = "^"; // Управляющий символ определяющий ось и направление вращения
    StreamS.append(QString::number(ui->spinBox1->value())); //скорость этого вращения
    // добавляется к управляющему символу, полный сигнал состоит из двух символов
    ui->textEdit->append(StreamS); // вывод (проверка) команды

    // передача команды в порт
    if (arduino_serial.isWritable()) {
           arduino_serial.write(StreamS.toUtf8()); // непосредственно отправка в порт с предварит. конвертацией
            arduino_serial.waitForBytesWritten();
            while (arduino_serial.waitForReadyRead(1))
            {
                 arduino_serial.readAll();
            }

            QByteArray data;  // специальный тип QT для хранения последовательности байтов
                while (arduino_serial.waitForReadyRead(3)) {
                    // вместо холостого чтения накапливаем результат в переменную data
                    data.append(arduino_serial.readAll());
                }
            ui->textEdit_2->append(data);
       }     // в случае проблем
       else {
           QMessageBox::information(this, "Serial Port Error", "Couldn't write to Serial port.");
       }
}

void MainWindow::on_pushButton_4_pressed()
{
    StreamS = "V"; // Управляющий символ определяющий ось и направление вращения
    StreamS.append(QString::number(ui->spinBox1->value())); //скорость этого вращения
    // добавляется к управляющему символу, полный сигнал состоит из двух символов
    ui->textEdit->append(StreamS); // вывод (проверка) команды

    // передача команды в порт
    if (arduino_serial.isWritable()) {
           arduino_serial.write(StreamS.toUtf8()); // непосредственно отправка в порт с предварит. конвертацией
            arduino_serial.waitForBytesWritten();
            while (arduino_serial.waitForReadyRead(1))
            {
                 arduino_serial.readAll();
            }

            QByteArray data;  // специальный тип QT для хранения последовательности байтов
                while (arduino_serial.waitForReadyRead(3)) {
                    // вместо холостого чтения накапливаем результат в переменную data
                    data.append(arduino_serial.readAll());
                }
            ui->textEdit_2->append(data);
       }     // в случае проблем
       else {
           QMessageBox::information(this, "Serial Port Error", "Couldn't write to Serial port.");
       }
}


void MainWindow::on_pushButton_5_clicked() // подключение порта
{  
        //select name for our serial port from combobox
        arduino_portName = ui->comboBox->currentText();

       // if (arduino_serial.portName() != ui->comboBox->currentText())
        if (arduino_available == false)
        {
              arduino_serial.close();
              arduino_serial.setPortName(ui->comboBox->currentText());
              ui->textEdit->append(arduino_portName);
              ui->textEdit->append("Connection ON");
              ui->label_4->setText("Подключено");
              arduino_available = true;
        }
        arduino_serial.setBaudRate(QSerialPort::Baud115200);
        arduino_serial.setDataBits(QSerialPort::Data8);
        arduino_serial.setParity(QSerialPort::NoParity);
        arduino_serial.setStopBits(QSerialPort::OneStop);
        arduino_serial.setFlowControl(QSerialPort::NoFlowControl);
        arduino_serial.open(QSerialPort::ReadWrite);
}

void MainWindow::on_pushButton_6_clicked()
{
        //close
        if (arduino_available == true)
        {
        arduino_serial.close();
        ui->textEdit->append("Connection OFF");
        arduino_available = false;
        ui->label_4->setText("Не подключено");
        } else ui->textEdit->append("Nothing unconnect");
}


void MainWindow::on_pushButton_9_clicked()
{
    StreamS = "+0"; // Управляющий символ -- СТОП
       ui->textEdit->append(StreamS); // вывод (проверка) команды

    // передача команды в порт
    if (arduino_serial.isWritable()) {
           arduino_serial.write(StreamS.toUtf8()); // непосредственно отправка в порт с предварит. конвертацией
            arduino_serial.waitForBytesWritten();
            while (arduino_serial.waitForReadyRead(1))
            {
                 arduino_serial.readAll();
            }

            QByteArray data;  // специальный тип QT для хранения последовательности байтов
                while (arduino_serial.waitForReadyRead(3)) {
                    // вместо холостого чтения накапливаем результат в переменную data
                    data.append(arduino_serial.readAll());
                }
            ui->textEdit_2->append(data);
       }     // в случае проблем
       else {
           QMessageBox::information(this, "Serial Port Error", "Couldn't write to Serial port.");
       }
}

void MainWindow::on_action_6_triggered()
{

    static QPalette darkPalette;
    // Настраиваем палитру для цветовых ролей элементов интерфейса
    darkPalette.setColor(QPalette::Window, QColor(33, 33, 33));
    darkPalette.setColor(QPalette::WindowText, Qt::red);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(33, 33, 33));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::red);
    darkPalette.setColor(QPalette::ToolTipText, Qt::red);
    darkPalette.setColor(QPalette::Text, Qt::red);
    darkPalette.setColor(QPalette::Button, QColor(33, 33, 33));
    darkPalette.setColor(QPalette::ButtonText, Qt::red);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(220, 50, 56));
    darkPalette.setColor(QPalette::Highlight, QColor(220, 50, 56));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    // неактивные (отключенные кнопки)
    darkPalette.setColor(QPalette::Disabled, QPalette::Button, QColor(33, 33, 33));
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(100, 10, 12));

    // Устанавливаем данную палитру
    qApp->setPalette(darkPalette);
}

void MainWindow::on_action_7_triggered()
{
    // Для возврата к светлой палитре достаточно
    // будет установить стандартную палитру из темы оформления
    qApp->setPalette(style()->standardPalette());
}

void MainWindow::on_action_2_triggered() // Пункт меню "о программе"
{
    QMessageBox::information(this, "О программе", "StarGuidSystem, Author: P.L. Nering, 2020 \n V. 0.1.201024");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Изменение скоростей вращения двигателей и режимов

void MainWindow::on_dial_valueChanged(int value) // Крутилка скорость фокуса
{
    ui->spinBox->setValue(value);
}

void MainWindow::on_dial_2_valueChanged(int value) // Крутилка скорость оси DE
{
    ui->spinBox1->setValue(value);
}

void MainWindow::on_dial_3_valueChanged(int value) // Крутилка скорость оси RA
{
    ui->spinBox2->setValue(value);
}

void MainWindow::on_horizontalSlider_valueChanged(int value) // Слайдер переключения режимов ведения
{
    switch (value)
    {
     case 1: {ui->label_5->setText("нулевая (стоп)"); StreamS = "S0";} break;
     case 2: {ui->label_5->setText("звёзды"); StreamS = "S1";} break;
     case 3: {ui->label_5->setText("Луна"); StreamS = "S2";} break;
     case 4: {ui->label_5->setText("Солнце"); StreamS = "S3";}break;
    }
    ui->textEdit->append(StreamS); // вывод (проверка) команды

    // передача команды в порт
    if (arduino_serial.isWritable()) {
           arduino_serial.write(StreamS.toUtf8()); // непосредственно отправка в порт с предварит. конвертацией
            arduino_serial.waitForBytesWritten();
            while (arduino_serial.waitForReadyRead(1))
            {
                 arduino_serial.readAll();
            }

            QByteArray data;  // специальный тип QT для хранения последовательности байтов
                while (arduino_serial.waitForReadyRead(3)) {
                    // вместо холостого чтения накапливаем результат в переменную data
                    data.append(arduino_serial.readAll());
                }
            ui->textEdit_2->append(data);
       }     // в случае проблем
       else {
           QMessageBox::information(this, "Serial Port Error", "Couldn't write to Serial port.");
       }

}
//////////////////////////////////////////////////////////////////////////////////////////////////
// Фокус +
void MainWindow::on_pushButton_8_clicked()
{
    StreamS = "F"; // Управляющий символ определяющий что фокусируем
    StreamS.append(QString::number(ui->spinBox->value())); //скорость этого вращения
    StreamS.append("+"); //направление фокуса
    // добавляется к управляющему символу, полный сигнал состоит из трех символов
    ui->textEdit->append(StreamS); // вывод (проверка) команды

    // передача команды в порт
    if (arduino_serial.isWritable()) {
           arduino_serial.write(StreamS.toUtf8()); // непосредственно отправка в порт с предварит. конвертацией
            arduino_serial.waitForBytesWritten();
            while (arduino_serial.waitForReadyRead(1))
            {
                 arduino_serial.readAll();
            }

            QByteArray data;  // специальный тип QT для хранения последовательности байтов
                while (arduino_serial.waitForReadyRead(3)) {
                    // вместо холостого чтения накапливаем результат в переменную data
                    data.append(arduino_serial.readAll());
                }
            ui->textEdit_2->append(data);
       }     // в случае проблем
       else {
           QMessageBox::information(this, "Serial Port Error", "Couldn't write to Serial port.");
       }
}
// Фокус -
void MainWindow::on_pushButton_7_clicked()
{
    StreamS = "F"; // Управляющий символ определяющий что фокусируем
    StreamS.append(QString::number(ui->spinBox->value())); //скорость этого вращения
    StreamS.append("-"); //направление фокуса
    // добавляется к управляющему символу, полный сигнал состоит из трех символов
    ui->textEdit->append(StreamS); // вывод (проверка) команды

    // передача команды в порт
    if (arduino_serial.isWritable()) {
           arduino_serial.write(StreamS.toUtf8()); // непосредственно отправка в порт с предварит. конвертацией
            arduino_serial.waitForBytesWritten();
            while (arduino_serial.waitForReadyRead(1))
            {
                 arduino_serial.readAll();
            }

            QByteArray data;  // специальный тип QT для хранения последовательности байтов
                while (arduino_serial.waitForReadyRead(3)) {
                    // вместо холостого чтения накапливаем результат в переменную data
                    data.append(arduino_serial.readAll());
                }
            ui->textEdit_2->append(data);
       }     // в случае проблем
       else {
           QMessageBox::information(this, "Serial Port Error", "Couldn't write to Serial port.");
       }
}



////////////////// проверки
void MainWindow::on_pushButton_10_clicked()
{
     StreamS = "s";
    // передача команды в порт
     ui->textEdit->append(StreamS); // вывод (проверка) команды
    if (arduino_serial.isWritable()) {
           arduino_serial.write(StreamS.toUtf8()); // непосредственно отправка в порт с предварит. конвертацией
            arduino_serial.waitForBytesWritten();
            while (arduino_serial.waitForReadyRead(1))
            {
                 arduino_serial.readAll();
            }

            QByteArray data;  // специальный тип QT для хранения последовательности байтов
                while (arduino_serial.waitForReadyRead(3)) {
                    // вместо холостого чтения накапливаем результат в переменную data
                    data.append(arduino_serial.readAll());
                }
            ui->textEdit_2->append(data);
       }     // в случае проблем
       else {
           QMessageBox::information(this, "Serial Port Error", "Couldn't write to Serial port.");
       }
}
