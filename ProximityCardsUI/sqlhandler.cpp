#include "sqlhandler.h"
#include <QDebug>
#include <QSqlError>
#include <QMessageBox>

SqlHandler::SqlHandler(QString hostName, QString userName, QString password,
                       QString databaseName, QString tableName, QObject *parent)
    : QObject(parent),
      sqlDatabaseConnection_(new QSqlDatabase(QSqlDatabase::addDatabase("QMYSQL"))),
      tableIsReadyToUse_(false),
      hostName_(hostName),
      userName_(userName),
      password_(password),
      databaseName_(databaseName),
      tableName_(tableName)
{
}

SqlHandler::~SqlHandler() {
    if (sqlDatabaseConnection_) {
        sqlDatabaseConnection_->close();
        QSqlDatabase::removeDatabase("QMYSQL");
    }
}

void SqlHandler::init(void){
    if (sqlDatabaseConnection_) {
        sqlDatabaseConnection_->setHostName(hostName_); //"localhost", "root", "olhasham2012",
        sqlDatabaseConnection_->setUserName(userName_);
        sqlDatabaseConnection_->setPassword(password_);

        if (openDatabaseConnection()) {
           if (selectDatabase(databaseName_)) {
               if (selectTable(tableName_)) {
                   if (verifyTableFields(tableName_)) {
                        tableIsReadyToUse_ = true;
                   } else {
                       if (removeTable(tableName_)) {
                           if (createTable(tableName_))
                               tableIsReadyToUse_ = true;
                           else
                               tableIsReadyToUse_ = false;
                        } else
                           tableIsReadyToUse_ = false;
                   }
               } else {
                    if (createTable(tableName_))
                        tableIsReadyToUse_ = true;
                    else
                        tableIsReadyToUse_ = false;
               }
           } else {
                if (createDatabase(databaseName_)) {
                    if (selectDatabase(databaseName_)) {
                        if (createTable(tableName_)) {
                            tableIsReadyToUse_ = true;
                        } else {
                           tableIsReadyToUse_ = false;
                        }
                     } else {
                        tableIsReadyToUse_ = false;
                    }
                } else {
                    tableIsReadyToUse_ = false;
                }
           }
        } else {
            emit signalErrorMessage(tr("No connection to DB!"),
                                    tr("Check MySQL settings, username and password!"));
        }
    }
}

bool SqlHandler::openDatabaseConnection(void) {
    bool result = false;
    if (sqlDatabaseConnection_)
        result = sqlDatabaseConnection_->open();
    return result;
}

bool SqlHandler::selectDatabase(QString name) {
    bool result = false;
    if (query_.exec(QString("use ") + name)) {
        result = true;
    } else {
        qDebug() << query_.lastError().text() ;
    }
    return result;
}

bool SqlHandler::createDatabase(QString name) {
    bool result = false;
    if (query_.exec(QString("create database ") + name)) {
        result = true;
    } else {
        qDebug() << query_.lastError().text() ;
    }
    return result;
}

bool SqlHandler::selectTable(QString tableName) {
    bool result = false;
    if (query_.exec(QString("select * from ") + tableName)) {
        result = true;
    } else {
        qDebug() << query_.lastError().text() ;
    }
    return result;
}

bool SqlHandler::removeTable(QString tableName) {
    bool result = false;
    if (query_.exec(QString("drop table ") + tableName)) {
        result = true;
    } else {
        qDebug() << query_.lastError().text() ;
    }
    return result;
}

bool SqlHandler::verifyTableFields(QString tableName) {
    bool result = false;
    if (query_.exec(QString("select CARD_ID, CARD_NUMBER, CREATION_DATE from ") + tableName)) {
        result = true;
    } else {
        qDebug() << query_.lastError().text() ;
    }
    return result;
}

bool SqlHandler::createTable(QString tableName) {
    bool result = false;
    QString query = QString("create table %1  ("
                            "CARD_ID INT AUTO_INCREMENT PRIMARY KEY,"
                            "CARD_NUMBER VARCHAR(200),"
                            "CREATION_DATE DATETIME)").arg(tableName);
    if (query_.exec(query))
        result = true;
    else
        qDebug() << query_.lastError().text() ;
    return result;
}


bool SqlHandler::setDataToTable(QString data) {

    bool result = false;

    if (tableIsReadyToUse_) {
        QString query = QString("insert into %1  (CARD_NUMBER, CREATION_DATE) value (\"%2\", CURRENT_TIMESTAMP() )")
                .arg(tableName_).arg(data);
        if (query_.exec(query)) {
            result = true;
        } else {
            qDebug() << query_.lastError().text() ;
        }
    }
    return result;
}

bool SqlHandler::isSameDataInTable(QString data) {
    bool result = false;
    if (tableIsReadyToUse_) {
        QString query = QString("select * from %1  where CARD_NUMBER = \"%2\"")
                .arg(tableName_).arg(data);
        if (query_.exec(query)) {
            if (query_.size())
                result = true;
        } else {
            qDebug() << query_.lastError().text() ;
        }
    }
    return result;
}

