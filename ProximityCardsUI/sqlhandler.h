#ifndef SQLHANDLER_H
#define SQLHANDLER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <memory>
#include <QObject>
#include <QString>
#include <QMutex>

class SqlHandler : public QObject
{
    Q_OBJECT
public:
    explicit SqlHandler(QString hostName, QString userName, QString password,
                        QString databaseName, QString tableName, QObject *parent = nullptr);

    SqlHandler(SqlHandler &&other) {
        this->sqlDatabaseConnection_ = std::move(other.sqlDatabaseConnection_);
        this->query_ = other.query_;
        this->tableIsReadyToUse_ = other.tableIsReadyToUse_;
        this->hostName_ = other.hostName_;
        this->userName_ = other.userName_;
        this->password_ = other.password_;
        this->databaseName_ = other.databaseName_;
        this->tableName_ = other.tableName_;
    }

    SqlHandler & operator=(SqlHandler &&other) {
        this->sqlDatabaseConnection_ = std::move(other.sqlDatabaseConnection_);
        this->query_ = other.query_;
        this->tableIsReadyToUse_ = other.tableIsReadyToUse_;
        this->hostName_ = other.hostName_;
        this->userName_ = other.userName_;
        this->password_ = other.password_;
        this->databaseName_ = other.databaseName_;
        this->tableName_ = other.tableName_;
        return *this;
    }

    ~SqlHandler();

    void init(void);
    bool openDatabaseConnection(void);

    bool createDatabase(QString name);
    bool selectDatabase(QString name);
    bool createTable(QString tableName);
    bool selectTable(QString tableName);
    bool removeTable(QString tableName);
    bool verifyTableFields(QString tableName);

    bool setDataToTable(QString data);
    bool isSameDataInTable(QString data);

signals:
    void signalErrorMessage(const QString &title, const QString &message);
public slots:
private:
    std::unique_ptr<QSqlDatabase> sqlDatabaseConnection_;
    QSqlQuery query_;
    bool tableIsReadyToUse_;
    QString hostName_;
    QString userName_;
    QString password_;
    QString databaseName_;
    QString tableName_;
    QMutex mutex_;
};

#endif // SQLHANDLER_H
