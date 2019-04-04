#include "user_db.h"
#include <QString>
#include <QDir>
#include "debug.h"
#include "global.h"

#define STALKER_CONF_FILE		"/usr/local/stalker/stalker.conf"

userDB::userDB(int *flag)
{
    int retv = 0;
    bool b;

    m_dbOpened = 0;
    m_query = NULL;

    QString dbPath = QString (qgetenv("LIDARCAM_DB_PATH"));
    if ( dbPath.isEmpty() )
        dbPath = QString("./db");

    QString dbName = dbPath + "/" + USERDB_NAME;
    DEBUG() << "user DB path = " <<QDir::currentPath() ;
    DEBUG() << "user DB name = " << dbName;

    QDir dir(dbPath);
    if (dir.exists() == false) {
        QString abp = dir.absolutePath();
        DEBUG() << "db Path not exist";
        b = dir.mkpath(abp);
        if (b == false) {
            DEBUG() << "make path " << abp << "failed";
            *flag = -1;
        }
    }

    QFileInfo f(dbName);
    if (f.exists() == false) {
        DEBUG() << "db file not exist";
        retv = initDB(dbName);
    } else {
        DEBUG() << "db exists, make connection";
        retv = makeConnection(dbName);
    }

    if (retv) {
        qFatal("open db failed");
        *flag = retv;
    }

    m_dbOpened = 1;

#if 0
    // test code
    initQuery();
    struct Users u;
    u.loginName = "admin";
    retv = this->Users_query(&u, QRY_BY_KEY);
    DEBUG() << "query return " << retv;
    //test code done
#endif
}

userDB::~userDB()
{
    if (m_query)
        delete m_query;

    if (m_dbOpened) {
        m_db.close();
        m_dbOpened = 0;
    }
}

int userDB::makeConnection(QString &dbName)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbName);

    if (!m_db.open()) {
        DEBUG() << "open db failed " << m_db.lastError().text();
        return (-ERR_OPENDB);
    }

    return 0;
}

void userDB::initQuery()
{
    if (m_query)
        delete m_query;
    m_query = new QSqlQuery(m_db);
}

int userDB::initDB(QString &dbName)
{
    FILE *fp;
    char str[128];
    char *pName, *pValue;
    int retv = makeConnection(dbName);

    if (retv) {
        DEBUG() << "make connection failed";
        return (-ERR_CONNECTDB);
    }

    //create tables
    int t = TBL_FIRST + 1;
    while (t < TBL_LAST) {
        retv = this->createTable((DB_TABLES)t);
        if (retv) {
            DEBUG() << "create table " << t << " failed";
            return retv;
        }
        t = t + 1;
    }

    this->initQuery();
    retv = Users_addEntry(NULL); //add admin User
    retv = Cams_addEntry(NULL);  //add default camera setting

    struct Evidence e1;
    e1.index = 0;
    e1.evidNum = 1;
    e1.imageNum = 1;
    e1.ticketNum = 1;
    Evid_addEntry(&e1);

    struct Location l;
    l.index = CAMS_DEFAULT_INDEX;
    l.description = " ";
    l.speedLimit = "65";
    l.captureSpeed = "70";
    l.captureDistance = "50";
    l.roadCondition = "NORMAL";    
    l.numberOfLanes = 2;
    Loc_addEntry(&l);

    struct SysConfig c1;
    c1.index = 0;
#ifdef HH1
    c1.targetSort = 0;
    c1.units = 0;
    c1.direction = 0;
    c1.autoTrigger = 0;
    c1.speedTenths = 0;
    c1.rangeTenths = 0;
    c1.imageSpacing = 0;
    c1.pictureDist = 15;
    c1.frequency = 24.0;
    c1.bandwidth = 100.0;
    c1.radarPower = 1;
    c1.sensitivity = 1;
    c1.volume = 2;
    c1.brightness = 2;
    c1.serialNumber = QString("Not Defined");

    fp = fopen(STALKER_CONF_FILE,"r");
    if(!fp) {
  	fprintf(stderr, "Unable to open stalker.conf configuration file\n");
        return -1;
    }

    // Get configuration
    while (fgets(str, sizeof(str), fp))
    {
      if(str[0] == '#') // Comment
         continue;

      pName = strtok(str," \t\n");
      pValue = strtok(NULL," \t\n");
      while (pName && pValue)
      {
  	if (!strncmp(pName, SERIALNUMBER, 12)) {
          c1.serialNumber = QString::fromStdString(pValue);
	}
	pName = strtok(NULL," \t\n");
	pValue = strtok(NULL," \t\n");
      }
    }
#else
    c1.frames = 30;
#endif
    c1.backlightOff = 0;
    c1.powerOff = 0;
    c1.resolution = 2;  // High resolution
    c1.imagesPerFile = 1;
    c1.preBuf = 5;     // 5 seconds
    c1.postBuf = 20;
    c1.radarH = 4.0;
    c1.targetH = 3.0;
    c1.distFrR = 9.0;
    c1.lensFocal = 25;
    c1.pictureDist = "130";

    Conf_addEntry(&c1);

    struct Administration a1;
    a1.index = 0;
    a1.userLogin = false;
    a1.compression = false;
    a1.autoDelete = false;
    a1.encryption = false;
    a1.password = ADMINISTRATION_PASSWORD;
    a1.dateFormat = 0;  // YYYY-MM-DD
    a1.language = 0;    // English
    a1.expiration = "";
    a1.authority = "";
    a1.certification = "";
    a1.serviceDate = "";
    a1.metaData1 = "%0 %3 %4 %5 ";
    a1.metaData2 = "%1 %2 %6 ";
    a1.metaPosition = WATERMARK_TOPLEFT;
    a1.usrAccAdmin = 0;     // None
    a1.usrAccTransfer = 0;  // None
    a1.usrAccDelete = 0;    // None
    Admin_addEntry(&a1);

    struct Network n1;
    n1.index = 0;
    n1.discovery = 0;       // DHCP
    n1.ipAddr = 0x0A000501;
    n1.subnetMask = 0xFFFF0000;
    n1.gateway = 0x0A00FFFE;
    n1.dns1 = 0;
    n1.dns2 = 0;
    Network_addEntry(&n1);

#ifdef HH1
    //Need to get this data from the default config file for each unit
    struct Sensor s1;
    s1.index = 0;
    s1.i2cDevice = "/dev/i2c-1";
    s1.magXmax = 738.0;
    s1.magXmin = -58.0;
    s1.magYmax = 79.0;
    s1.magYmin = -601.0;
    s1.magZmax = 2373.0;
    s1.magZmin = 1584.0;
    s1.magThetaX = s1.magThetaY = s1.magThetaZ = 0.0;
    s1.accXmax = 16235.0;
    s1.accXmin = -16820.0;
    s1.accYmax = 16254.0;
    s1.accYmin = -15832.0;
    s1.accZmax = 16216.0;
    s1.accZmin = -16404.0;
    s1.accThetaX = s1.accThetaY = s1.accThetaZ = 0.0;

    fp = fopen(STALKER_CONF_FILE,"r");
    if (!fp)
    {
        fprintf(stderr, "Unable to open stalker.conf configuration file\n");
        return -1;
    }

    // Get configuration
    while (fgets(str, sizeof(str), fp))
    {
      if(str[0] == '#') // Comment
         continue;

      pName = strtok(str," \t\n");
      pValue = strtok(NULL," \t\n");
      while (pName && pValue)
      {
          if (!strncmp(pName, MAG_XMAX, 8))
                  s1.magXmax = atof(pValue);
          else if (!strncmp(pName, MAG_XMIN, 8))
                  s1.magXmin = atof(pValue);
          else if (!strncmp(pName, MAG_YMAX, 8))
                  s1.magYmax = atof(pValue);
          else if (!strncmp(pName, MAG_YMIN, 8))
                  s1.magYmin = atof(pValue);
          else if (!strncmp(pName, MAG_ZMAX, 8))
                  s1.magZmax = atof(pValue);
          else if (!strncmp(pName, MAG_ZMIN, 8))
                  s1.magZmin = atof(pValue);
          else if (!strncmp(pName, MAG_THETA_X, 11))
                  s1.magThetaX = atof(pValue);
          else if (!strncmp(pName, MAG_THETA_Y, 11))
                  s1.magThetaY = atof(pValue);
          else if (!strncmp(pName, MAG_THETA_Z, 11))
                  s1.magThetaZ = atof(pValue);
          else if (!strncmp(pName, ACC_XMAX, 8))
                  s1.accXmax = atof(pValue);
          else if (!strncmp(pName, ACC_XMIN, 8))
                  s1.accXmin = atof(pValue);
          else if (!strncmp(pName, ACC_YMAX, 8))
                  s1.accYmax = atof(pValue);
          else if (!strncmp(pName, ACC_YMIN, 8))
                  s1.accYmin = atof(pValue);
          else if (!strncmp(pName, ACC_ZMAX, 8))
                  s1.accZmax = atof(pValue);
          else if (!strncmp(pName, ACC_ZMIN, 8))
                  s1.accZmin = atof(pValue);
          else if (!strncmp(pName, ACC_THETA_X, 11))
                  s1.accThetaX = atof(pValue);
          else if (!strncmp(pName, ACC_THETA_Y, 11))
                  s1.accThetaY = atof(pValue);
          else if (!strncmp(pName, ACC_THETA_Z, 11))
                  s1.accThetaZ = atof(pValue);
          pName = strtok(NULL," \t\n");
          pValue = strtok(NULL," \t\n");
        }
    }

    fclose(fp);

    Sensor_addEntry(&s1);
#endif

    return 0;
}

int userDB::createLocSettings()
{

    return 0;
}

int userDB::createLidarSettings()
{

    return 0;
}


//
// public APIs
//
int userDB::queryLogin(QString &loginName, QString &passWord)
{
    if (!m_dbOpened)
        return -1;

    QSqlQuery qry(m_db);

    //query the Users table
    qry.prepare("SELECT id from Users where loginName = :loginName");
    qry.bindValue(":loginName", loginName);
    if (!qry.exec()) {
        DEBUG() << "query Users failed: " << qry.lastError();
        return -1;
    }
    if (qry.driver()->hasFeature(QSqlDriver::QuerySize)) {
        if (qry.size() != 1) {
            return qry.size();
        }
    } else {
        qry.last();
        int n = qry.at() + 1;
        if (n != 1)
            return n;
    }
    qry.first();
    int id = qry.value(0).toInt();

    //query the LoginInfo table
    qry.prepare("SELECT password from LoginInfo where id = :id");
    qry.bindValue(":id", id);
    if (!qry.exec()) {
        DEBUG() << "query LoginInfo failed: " << qry.lastError();
        return -1;
    }
    qry.next();
    QString pw = qry.value(0).toString();
    if (pw != passWord) {
        return 0;
    }

    return 1;
}

int userDB::queryNameFromLogin(QString &loginName, QString &userName)
{
    if (!m_dbOpened)
        return -1;

	DEBUG() << "Loginname " << loginName << "userName " << userName;
	
    QSqlQuery qry(m_db);

    qry.first();

    //query the Users table
    qry.prepare("SELECT * from Users where loginName = :loginName");
	printf("%s(%d) \n\r", __FILE__, __LINE__);

    qry.bindValue(":loginName", loginName);
	printf("%s(%d) \n\r", __FILE__, __LINE__);

    if (!qry.exec()) {
        DEBUG() << "query Users failed: " << qry.lastError();
        return -1;
    }
	printf("%s(%d) \n\r", __FILE__, __LINE__);

    int id = qry.value(0).toInt();
	printf("%s(%d) \n\r", __FILE__, __LINE__);
    userName = qry.value(1).toString();
	printf("%s(%d) \n\r", __FILE__, __LINE__);
    return id;
}

int userDB::addEntry(DB_TABLES table, DBStruct *dataType) {
    int retv = 0;

    switch (table) {
    case TBL_USERS:
        retv = Users_addEntry((struct Users *)dataType);
        break;
    case TBL_LOCATION:
        retv = Loc_addEntry((struct Location *)dataType);
        break;
    case TBL_CAMERA_SETTING:
        retv = Cams_addEntry((struct CameraSetting *)dataType);
        break;
    case TBL_CONFIGURATION:
        retv = Conf_addEntry((struct SysConfig *)dataType);
        break;
    case TBL_EVIDENCE:
        retv = Evid_addEntry((struct Evidence *)dataType);
        break;
    case TBL_ADMIN:
        retv = Admin_addEntry((struct Administration *)dataType);
        break;
    case TBL_NETWORK:
        retv = Network_addEntry((struct Network *)dataType);
        break;
#ifdef HH1
    case TBL_SENSOR:
        retv = Sensor_addEntry((struct Sensor *)dataType);
        break;
#endif
    default:
        retv = -ERR_ADD_ENTRY;
        break;

    }

    return retv;
}

int userDB::deleteEntry(DB_TABLES table, DBStruct *dataType, DB_QUERY_FLAG flag) {
    int retv = 0;
    switch (table) {
    case TBL_USERS:
        retv = Users_delEntry((struct Users *) dataType, flag);
        break;
    case TBL_LOCATION:
        retv = Loc_delEntry((struct Location *) dataType, flag);
        break;
    case TBL_CAMERA_SETTING:
        retv = Cams_delEntry((struct CameraSetting *)dataType, flag);
        break;
    case TBL_CONFIGURATION:
        retv = Conf_delEntry((struct SysConfig *)dataType, flag);
        break;
    case TBL_EVIDENCE:
        retv = Evid_delEntry((struct Evidence *)dataType, flag);
        break;
    case TBL_ADMIN:
        retv = Admin_delEntry((struct Administration *)dataType, flag);
        break;
    case TBL_NETWORK:
        retv = Network_delEntry((struct Network *)dataType, flag);
        break;
#ifdef HH1
    case TBL_SENSOR:
        retv = Sensor_delEntry((struct Sensor *)dataType, flag);
        break;
#endif
    default:
        retv = -ERR_DEL_ENTRY;
        break;
    }

    return retv;
}

int userDB::updateEntry(DB_TABLES table, DBStruct *dataType, DB_QUERY_FLAG flag) {
    int retv = 0;

    switch (table) {
    case TBL_USERS:
        retv = Users_updateEntry((struct Users *) dataType, flag);
        break;
    case TBL_LOCATION:
        retv = Loc_updateEntry((struct Location *) dataType, flag);
        break;
    case TBL_CAMERA_SETTING:
        retv = Cams_updateEntry((struct CameraSetting *)dataType, flag);
        break;
    case TBL_CONFIGURATION:
        retv = Conf_updateEntry((struct SysConfig *)dataType, flag);
        break;
    case TBL_EVIDENCE:
        retv = Evid_updateEntry((struct Evidence *)dataType, flag);
        break;
    case TBL_ADMIN:
        retv = Admin_updateEntry((struct Administration *)dataType, flag);
        break;
    case TBL_NETWORK:
        retv = Network_updateEntry((struct Network *)dataType, flag);
        break;
#ifdef HH1
    case TBL_SENSOR:
        retv = Sensor_updateEntry((struct Sensor *)dataType, flag);
        break;
#endif
    default:
        retv = -ERR_UPDATE_ENTRY;
        break;
    }

    return retv;
}

int userDB::queryEntry(DB_TABLES table, DBStruct *dataType, DB_QUERY_FLAG flag) {
    int retv;
    initQuery();

    switch (table) {
    case TBL_USERS:
        retv = Users_query((struct Users *)dataType, flag);
        break;
    case TBL_LOCATION:
        retv = Loc_query((struct Location *)dataType, flag);
        break;
    case TBL_CAMERA_SETTING:
        retv = Cams_query((struct CameraSetting *)dataType, flag);
        break;
    case TBL_CONFIGURATION:
        retv = Conf_query((struct SysConfig *)dataType, flag);
        break;
    case TBL_EVIDENCE:
        retv = Evid_query((struct Evidence *)dataType, flag);
        break;
    case TBL_ADMIN:
        retv = Admin_query((struct Administration *)dataType, flag);
        break;
    case TBL_NETWORK:
        retv = Network_query((struct Network *)dataType, flag);
        break;
#ifdef HH1
    case TBL_SENSOR:
        retv = Sensor_query((struct Sensor *)dataType, flag);
        break;
#endif
    default:
        retv = -ERR_QUERY_ENTRY;
        break;

    }

    m_firstEntry = 0;

    return retv;
}

int userDB::getNextEntry(DB_TABLES table, DBStruct *dataType) {
 
    switch (table) {
    case TBL_USERS:
      (void)Users_getNextEntry((struct Users *)dataType);
        break;
    case TBL_LOCATION:
      (void)Loc_getNextEntry((struct Location *)dataType);
        break;
    case TBL_CAMERA_SETTING:
      (void)Cams_getNextEntry((struct CameraSetting *)dataType);
        break;
    case TBL_CONFIGURATION:
      (void)Conf_getNextEntry((struct SysConfig *)dataType);
        break;
    case TBL_EVIDENCE:
      (void)Evid_getNextEntry((struct Evidence *)dataType);
        break;
    case TBL_ADMIN:
      (void)Admin_getNextEntry((struct Administration *)dataType);
        break;
    case TBL_NETWORK:
      (void)Network_getNextEntry((struct Network *)dataType);
        break;
#ifdef HH1
    case TBL_SENSOR:
      (void)Sensor_getNextEntry((struct Sensor *)dataType);
        break;
#endif
    default:
      //        retv = -ERR_QUERY_ENTRY;
      break;

    }

    return 0;
}


//
// private functions
//

int userDB::createTable(DB_TABLES table)
{
    // DB naming conviention
    // tables:  Table_Name,  User_Levels
    // table column: FirstName, LoginName

    int retv = 0;
    QString query = "";
    switch (table)
    {
    case TBL_USERS:       //create table users here
        retv = this->Users_createTbl();
        break;
    case TBL_LOCATION:
        retv = this->Loc_createTbl();
        break;
    case TBL_CAMERA_SETTING:
        retv = this->Cams_createTbl();
        break;
    case TBL_QUICK_CODE:
        query = "CREATE TABLE Quick_Code "
                "(Code TEXT PRIMARY KEY, "
                "LoginName TEXT)";    //REFERENCES Users(LoginName)
        break;
    case TBL_USER_PERM:
        query = "CREATE TABLE User_Perm "
                "(UserLevel NUMERIC PRIMARY KEY, "
                "Admin1 BOOLEAN NOT NULL, Upload1 BOOLEAN NOT NULL, Delete1 BOOLEAN NOT NULL, "
                "Admin2 BOOLEAN NOT NULL, Upload2 BOOLEAN NOT NULL, Delete2 BOOLEAN NOT NULL, "
                "Admin3 BOOLEAN NOT NULL, Upload3 BOOLEAN NOT NULL, Delete3 BOOLEAN NOT NULL, "
                "Admin4 BOOLEAN NOT NULL, Upload4 BOOLEAN NOT NULL, Delete4 BOOLEAN NOT NULL)";
        break;
    case TBL_USER_LEVEL:
        query = "CREATE TABLE User_Level "
                "(UserLevel NUMERIC PRIMARY KEY, "
                "LevelTitle TEXT NOT NULL)";
        break;
    case TBL_CONFIGURATION:
        retv = Conf_createTbl();
        break;
    case TBL_CALIB_DATA:
        query = "CREATE TABLE Calib_Data "
                "(Expire NUMERIC PRIMARY KEY, "
                "Authority TEXT NOT NULL, "
                "Certification TEXT NOT NULL)";
        break;
    case TBL_EVIDENCE:
        retv = Evid_createTbl();
        break;
    case TBL_ADMIN:
        retv = Admin_createTbl();
        break;
    case TBL_NETWORK:
        retv = Network_createTbl();
        break;
#ifdef HH1
    case TBL_SENSOR:
        retv = Sensor_createTbl();
        break;
#endif
    default:
        DEBUG() << "Invalid table " << table;
        return (-ERR_INVAL);
    }

#if 0
    if (table != TBL_USERS && table != TBL_LOCATION) {
        QSqlQuery dbquery(m_db);
        if (!dbquery.exec(query))
        {
            DEBUG() << "Create table failed: number " << table;
            DEBUG() << dbquery.lastError();
            return (-ERR_CREATE_TBL);
        }
    }
#endif

    Q_ASSERT(!retv);

    return 0;
}
