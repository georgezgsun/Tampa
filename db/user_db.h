#ifndef USER_DB_H
#define USER_DB_H

#include <QtSql>
#include <QFileInfo>
//#include <QDebug>
#include "db_types.h"

#define USERDB_NAME     "lidarSystemDB.db"
#define SUPPER_USER     1
#define NORMAL_USER     2

#define ERR_OPENDB          1
#define ERR_CONNECTDB       2
#define ERR_CREATE_TBL      3
#define ERR_INVAL           4       // invalid parameters
#define ERR_ADD_ENTRY       5
#define ERR_QUERY_ENTRY     6
#define ERR_QUERY_EXEC      7       //execution failed
#define ERR_DEL_ENTRY       8
#define ERR_UPDATE_ENTRY    9
#define ERR_UPDATE_PREPARE  10
#define ERR_UPDATE_EXEC     11

#define MAX_LOCATION_ENTRIES    200
#define CAMS_DEFAULT_INDEX  (MAX_LOCATION_ENTRIES+1)

#define ADMINISTRATION_PASSWORD      "123123"

#define IN
#define OUT

class userDB
{
private:
    int m_dbOpened;
    QSqlDatabase m_db;
    QSqlQuery *m_query;

    int initDB(QString &);
    int makeConnection(QString &);
    void initQuery();
    int createTable(DB_TABLES table);

    bool m_firstEntry;  //if first entry is retrieved, set to 0 after exec a query

    //
    //for individual tables
    //

    //for table Users
    int Users_createTbl();
    int Users_addEntry(struct Users *);
    int Users_delEntry(struct Users *, DB_QUERY_FLAG flag);
    int Users_query(struct Users *, DB_QUERY_FLAG flag);
    int Users_getNextEntry(struct Users *);
    int Users_updateEntry(struct Users *, DB_QUERY_FLAG flag);

    //for table Location
    int Loc_createTbl();
    int Loc_addEntry(struct Location *);
    int Loc_delEntry(struct Location *, DB_QUERY_FLAG flag);
    int Loc_query(struct Location *, DB_QUERY_FLAG flag);
    int Loc_getNextEntry(struct Location *);
    int Loc_updateEntry(struct Location *, DB_QUERY_FLAG flag);

    //for table CameraSettings
    int Cams_createTbl();
    int Cams_addEntry(struct CameraSetting *);
    int Cams_delEntry(struct CameraSetting *, DB_QUERY_FLAG flag);
    int Cams_query(struct CameraSetting *, DB_QUERY_FLAG flag);
    int Cams_getNextEntry(struct CameraSetting *);
    int Cams_updateEntry(struct CameraSetting *, DB_QUERY_FLAG flag);

    // For table Evidence
    int Evid_createTbl();
    int Evid_addEntry(struct Evidence *);
    int Evid_delEntry(struct Evidence *, DB_QUERY_FLAG flag);
    int Evid_query(struct Evidence *, DB_QUERY_FLAG flag);
    int Evid_getNextEntry(struct Evidence *);
    int Evid_updateEntry(struct Evidence *, DB_QUERY_FLAG flag);

    // For table Configuration
    int Conf_createTbl();
    int Conf_addEntry(struct SysConfig *);
    int Conf_delEntry(struct SysConfig *, DB_QUERY_FLAG flag);
    int Conf_query(struct SysConfig *, DB_QUERY_FLAG flag);
    int Conf_getNextEntry(struct SysConfig *);
    int Conf_updateEntry(struct SysConfig *, DB_QUERY_FLAG flag);

    // For table Administration
    int Admin_createTbl();
    int Admin_addEntry(struct Administration *);
    int Admin_delEntry(struct Administration *, DB_QUERY_FLAG flag);
    int Admin_query(struct Administration *, DB_QUERY_FLAG flag);
    int Admin_getNextEntry(struct Administration *);
    int Admin_updateEntry(struct Administration *, DB_QUERY_FLAG flag);

    // For table Network
    int Network_createTbl();
    int Network_addEntry(struct Network *);
    int Network_delEntry(struct Network *, DB_QUERY_FLAG flag);
    int Network_query(struct Network *, DB_QUERY_FLAG flag);
    int Network_getNextEntry(struct Network *);
    int Network_updateEntry(struct Network *, DB_QUERY_FLAG flag);

#ifdef HH1
    // For table Sensor
    int Sensor_createTbl();
    int Sensor_addEntry(struct Sensor *);
    int Sensor_delEntry(struct Sensor *, DB_QUERY_FLAG flag);
    int Sensor_query(struct Sensor *, DB_QUERY_FLAG flag);
    int Sensor_getNextEntry(struct Sensor *);
    int Sensor_updateEntry(struct Sensor *, DB_QUERY_FLAG flag);
#endif

    //create tables
    int createLocSettings();
    int createLidarSettings();

    //int addUserEntry(DBStruct *);  //if param is NULL, add admin user

public:
    userDB(int *flag);
    ~userDB();

    // ***** general purpose APIs
    int addEntry(DB_TABLES table, IN DBStruct *dataType);  //dataType is a struct of the table, return success (0) or fail (!=0)
    int deleteEntry(DB_TABLES table, IN DBStruct *dataType, DB_QUERY_FLAG flag=QRY_BY_KEY);
    int updateEntry(DB_TABLES table, IN DBStruct *dataType, DB_QUERY_FLAG flag=QRY_BY_KEY); //dataType has a key for the entry, query and replace the entry
    int queryEntry(DB_TABLES table, IN DBStruct *dataType, DB_QUERY_FLAG flag=QRY_BY_MULTI_FIELDS); //criteron is the query filed, dataType contains the value,
    int getNextEntry(DB_TABLES table, OUT DBStruct *dataType);

    //temp[orary api

    int queryLogin (QString &loginName, QString &passWord);
    int queryNameFromLogin(IN QString& loginName, OUT QString& userName);

};

#endif // USER_DB_H
