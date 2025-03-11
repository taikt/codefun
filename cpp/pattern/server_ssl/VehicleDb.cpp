#include <iostream>
#include <unordered_map>
#include "VehicleDb.h"

#define BEST_MATCH_RATE (0.80) // TO-DO

// related to get processig time
bool _qpcInited = false;
double PCFreq = 0.0;
__int64 CounterStart = 0;
double _avgdur = 0;
static void InitCounter();
static double CLOCK();
static double avgdur(double newdur);


extern std::unordered_map<string, uint32_t> configured_param;
extern std::unordered_map<uint32_t, string> sessionid_user_map;
extern std::unordered_map<string, uint32_t> partial_match;

VehicleDb::VehicleDb(const char* db_file):
    _db_file(db_file)
{
}

int VehicleDb::findBestmatch(char* first, u_int32_t first_len, char* second)
{
    int m = (int)(first_len);
    int n = (int)(strlen(second));

    int distance = 0;
    double similarity = 0.0;

    int cmpResult = 1;

    double max_length = (double)max(m, n);
    //int T[m + 1][n + 1];
    int T[30][30] = { 0, };

    {
        if (max_length > 0) {

            for (int i = 1; i <= m; i++) {
                T[i][0] = i;
            }

            for (int j = 1; j <= n; j++) {
                T[0][j] = j;
            }

            for (int i = 1; i <= m; i++) {
                for (int j = 1; j <= n; j++) {
                    int weight = first[i - 1] == second[j - 1] ? 0 : 1;
                    T[i][j] = min(min(T[i - 1][j] + 1, T[i][j - 1] + 1), T[i - 1][j - 1] + weight);
                }
            }

            distance = T[m][n];

            //printf("max_length: %.f, distance: %d\n", max_length, distance);

            //similarity = ((max_length - distance) / max_length) * 100.0;
            similarity = ((max_length - distance) / max_length);

            //std::cout << "similarity: " << similarity << std::endl;

            // TODO: use configure threshold
            if (similarity * 100 >= configured_param[PARTIAL_THRESHOLD])
            {
                //printf("\nmax_length: %.f, distance: %d\n", max_length, distance);
                //printf("Found best match result\n");
                cmpResult = 0;
            }
        }
        else
        {
            similarity = 0;
            return -1;
        }
    }

    //printf("======= cmpResult: %d\n", cmpResult);
    return cmpResult;
}

bool VehicleDb::create_db() {
    u_int32_t flags; /* database open flags */
    int ret; /* function return value */

#if 0
    DB_ENV* dbenv = NULL;

    /* Create the environment handle. */
    if ((ret = db_env_create(&dbenv, 0)) != 0) {
        printf("DB Env Create Error\n");
        return false;
    }

    dbenv->set_errfile(dbenv, stderr);

    if ((ret = dbenv->open(dbenv, NULL,
        DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG |
        DB_INIT_MPOOL | DB_INIT_TXN | DB_RECOVER | DB_THREAD,
        0)) != 0) {
        (void)dbenv->close(dbenv, 0);
        printf("DB Env Open Error\n");
        return false;
    }
#endif

    /* Initialize the structure. This
     * database is not opened in an environment,
     * so the environment pointer is NULL. */
    ret = db_create(&dbp, NULL, 0);
    if (ret != 0) {
        /* Error handling goes here */
        printf("DB Create Error\n");
        return false;
    }
    
    if ((ret = dbp->set_flags(dbp, DB_DUP | DB_DUPSORT)) != 0)
    {
        /* Error handling goes here */
        printf("DB set flag Error\n");
        return -1;
    }

#ifdef PARTIAL_MATCH
    u_int32_t gbytesp = 2;  //2G
    u_int32_t bytesp = 0;
    int ncachep = 1;
    dbp->set_cachesize(dbp, gbytesp, bytesp, ncachep);

    gbytesp = 0;
    bytesp = 0;
    ncachep = 0;
    dbp->get_cachesize(dbp, &gbytesp, &bytesp, &ncachep);
    printf("Get BDB cache size %d %d %d\n", gbytesp, bytesp, ncachep);

    /* Database open flags */
    flags = DB_CREATE | DB_THREAD; /* If the database does not exist,
     * create it.*/
     /* open the database */
    ret = dbp->open(dbp, /* DB structure pointer */
        NULL, /* Transaction pointer */
        _db_file, /* On-disk file that holds the database. */
        NULL, /* Optional logical database name */
        DB_BTREE, /* Database access method */
        flags, /* Open flags */
        0); /* File mode (using defaults) */
    if (ret != 0) {
        /* Error handling goes here */
        printf("DB Open Error\n");
        return false;
    }
#else
    /* Database open flags */
    flags = DB_CREATE; /* If the database does not exist,
     * create it.*/
     /* open the database */
    ret = dbp->open(dbp, /* DB structure pointer */
        NULL, /* Transaction pointer */
        _db_file, /* On-disk file that holds the database. */
        NULL, /* Optional logical database name */
        DB_HASH, /* Database access method */
        flags, /* Open flags */
        0); /* File mode (using defaults) */
    if (ret != 0) {
        /* Error handling goes here */
        printf("DB Open Error\n");
        return false;
    }
#endif
}

#ifdef OLD_PLATE_QUERY  
bool VehicleDb::retrieve_vehicle_info(string _key, char* vehicle_info) {
    DBT key, data, prefix;
    char DBRecord[BUFFER_SIZE];
    char prefixString[2] = { 0, };
    DBC* cursorp;
    int bestMatchResult = 0;
    int ret;

    printf("retrieve_vehicle_info: plate :%s\n", _key.c_str());
    /* Zero out the DBTs before using them. */
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    memset(&prefix, 0, sizeof(DBT));

    key.data = (void*)_key.c_str();
    key.size = (u_int32_t)(_key.size() + 1);
    data.data = DBRecord;
    data.ulen = sizeof(DBRecord);
    data.flags = DB_DBT_USERMEM;

    char* c_key = (char*)_key.c_str();
    
    prefixString[0] = c_key[0];
    prefix.data = prefixString;
    prefix.size = 2;

    if (dbp->get(dbp, NULL, &key, &data, 0) != DB_NOTFOUND) {
        strncpy_s(vehicle_info, BUFFER_SIZE, (char*)data.data, BUFFER_SIZE);

        // check duplicate key
        dbp->cursor(dbp, NULL, &cursorp, 0);
        // move cursor to current key
        ret = cursorp->get(cursorp, &key, &data, DB_SET);

        if (ret != DB_NOTFOUND)
        {
            // move cursor to next duplicate record
            if ((ret = cursorp->get(cursorp, &key,
                &data, DB_NEXT_DUP)) == 0) {

                printf("Found dup record ->%s\n", (char*)key.data);

                strncpy_s(vehicle_info, BUFFER_SIZE, (char*)data.data, BUFFER_SIZE);
            }
        }
        return true;
    }
    else {
#ifdef PARTIAL_MATCH
        printf("not found exact match result.\n");

        dbp->cursor(dbp, NULL, &cursorp, 0);

     // DB_SET_RANGE is supported by only  btree  
        ret = cursorp->get(cursorp, &prefix, &data, DB_SET_RANGE);

        if (ret == DB_NOTFOUND)
        {
            printf("SET RANGE fail\n");
            dbp->cursor(dbp, NULL, &cursorp, 0);
        }

#pragma warning(suppress : 4996)
        char* ptr = strtok((char*)data.data, "\n");
        printf("Move to cursor ->%s\n", ptr);
        memset(&data, 0, sizeof(DBT));

        double start = CLOCK();
        int cnt = 0;
        double matchStart;
        double matchDur;
        double matchAvg;
        int recordCnt = 0;

        while ((ret = cursorp->get(cursorp, &key,
            &data, DB_NEXT)) == 0) {
            /* Do interesting things with the DBTs here. */
            //printf("####move cursor to ->%s\n", (char*)key.data);       
            // stop searching if current record has different prefix     
            char keyData = ((char*)key.data)[0];
            if ((keyData != prefixString[0])||(cnt >= 2))
            {
                printf("Different prefix. stop best matching key:%c, prefixString: %c \n", keyData, prefixString[0]);
                break;
            }
            recordCnt++;
            matchStart = CLOCK();
            bestMatchResult = findBestmatch((char*)_key.c_str(), _key.length(), (char*)key.data);
            matchDur = CLOCK() - matchStart;
            matchAvg = avgdur(matchDur);

            if (bestMatchResult == 0)
            {
                printf("Found best match ->%s\n", (char*)key.data);
                strncpy_s(vehicle_info, BUFFER_SIZE, (char*)data.data, BUFFER_SIZE);
                cnt++;
            }
        }
        if (ret != DB_NOTFOUND) {
            /* Error handling goes here */
            printf("Finish DB find\n");
        }
        double dur = CLOCK() - start;
        //printf("last key:%s \n", (char*)key.data);
        printf("%d found, search time %f ms, %f s, avg time best match %f ms\n", cnt, dur, dur / 1000.0, matchAvg);
        printf("Record count: %d \n", recordCnt);

        if (cnt == 0)
        {
            return false;
        }
        else
        {           
            return true;
        }

#else
        return false;
#endif
    }
}
#else


uint8_t VehicleDb::retrieve_vehicle_info(char* _key, u_int32_t _key_len, char** vehicle_info , uint8_t  max_partial_cnt, uint32_t sessionId) {
    DBT key, data, prefix;
    char DBRecord[BUFFER_SIZE] = { 0, };
    char prefixString[2] = { 0, };
    DBC* cursorp;
    int bestMatchResult = 0;
    int ret;       
    
    uint8_t  num_vehicle = 0;
    uint32_t vehicle_info_len;

    printf("====retrieve_vehicle_info: plate :%s len:%d\n", _key, _key_len);
    
    /* Zero out the DBTs before using them. */
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    memset(&prefix, 0, sizeof(DBT));
    
    key.data = (void*)_key;
    key.size = (u_int32_t)(_key_len + 1);
    data.data = DBRecord;
    data.ulen = sizeof(DBRecord);
    data.flags = DB_DBT_USERMEM;

    prefixString[0] = _key[0];
    prefix.data = prefixString;
    prefix.size = 2;

    if (dbp->get(dbp, NULL, &key, &data, 0) != DB_NOTFOUND) {
        vehicle_info_len = (strlen((char*)data.data) + 1);
        // fill vehicle length
        memcpy(*vehicle_info, &vehicle_info_len, 4);
        *vehicle_info += 4;

        // fill vehicle info
        char* ptr = (char*)data.data;
        memcpy(*vehicle_info, ptr, vehicle_info_len);
        //printf("Exact Key Find ->%s, length: %d\n", ptr, vehicle_info_len);

        *vehicle_info = (*vehicle_info) + vehicle_info_len;      

        num_vehicle = 1; 

        // check duplicate key
        dbp->cursor(dbp, NULL, &cursorp, 0);
        // move cursor to current key
        ret = cursorp->get(cursorp, &key, &data, DB_SET);

        if (ret != DB_NOTFOUND)
        {
            // move cursor to next duplicate record
            while ((ret = cursorp->get(cursorp, &key,
                &data, DB_NEXT_DUP)) == 0) {

                if (num_vehicle == max_partial_cnt)
                {
                    printf("Reach max count\n");
                    break;
                }

                printf("Found dup record ->%s\n", (char*)key.data);

                vehicle_info_len = (strlen((char*)data.data) + 1);
                // fill vehicle length
                memcpy(*vehicle_info, &vehicle_info_len, 4);
                *vehicle_info += 4;

                // fill vehicle info
                memcpy(*vehicle_info, (char*)data.data, vehicle_info_len);
                // move out pointer
                *vehicle_info = (*vehicle_info) + vehicle_info_len;
                num_vehicle++;
            }
        }
    }
    else {
        printf("not found exact match result.\n");

        dbp->cursor(dbp, NULL, &cursorp, 0);  

        ret = cursorp->get(cursorp, &prefix, &data, DB_SET_RANGE);

        if (ret == DB_NOTFOUND)
        {
            printf("SET RANGE fail\n");
            dbp->cursor(dbp, NULL, &cursorp, 0);
        }

#pragma warning(suppress : 4996)
        char* ptr = strtok((char*)data.data, "\n");
        printf("Move to cursor ->%s\n", ptr);
        memset(&data, 0, sizeof(DBT));

        double start = CLOCK();
        double matchStart;
        double matchDur;
        double matchAvg;
        int recordCnt = 0;

        while ((ret = cursorp->get(cursorp, &key,
            &data, DB_NEXT)) == 0) {
            /* Do interesting things with the DBTs here. */
            //printf("####move cursor to ->%s\n", (char*)key.data);   

            // stop searching if current record has different prefix     
            char keyData = ((char*)key.data)[0];
            if ((keyData != prefixString[0]) || (num_vehicle == max_partial_cnt))
            {
                printf("Different prefix. stop best matching key:%c, prefixString: %c \n", keyData, prefixString[0]);
                break;
            }
            recordCnt++;
            matchStart = CLOCK();
            bestMatchResult = findBestmatch(_key, _key_len, (char*)key.data);
            matchDur = CLOCK() - matchStart;
            matchAvg = avgdur(matchDur);

            if (bestMatchResult == 0)
            {
                printf("Found best match ->%s\n", (char*)key.data);

                vehicle_info_len = (strlen((char*)data.data) + 1);
                // fill vehicle length
                memcpy(*vehicle_info, &vehicle_info_len, 4);
                *vehicle_info += 4;

                // fill vehicle info
                memcpy(*vehicle_info, (char*)data.data, vehicle_info_len);
                // move out pointer
                *vehicle_info = (*vehicle_info) + vehicle_info_len;                
                num_vehicle++;                 
            }
        }
        if (ret != DB_NOTFOUND) {
            /* Error handling goes here */
            printf("Finish DB find\n");
        }   

        double dur = CLOCK() - start;
        //printf("last key:%s \n", (char*)key.data);
        printf("%d found, search time %f ms, %f s, avg time best match %f ms\n", num_vehicle, dur, dur / 1000.0, matchAvg);
        printf("Record count: %d \n", recordCnt);

        if (num_vehicle > 0)
        {
            // track partial match
            if (sessionid_user_map.count(sessionId)) {
                partial_match[sessionid_user_map[sessionId]]++;
            }
        }

    }

    return num_vehicle;
}
#endif

/***********************************************************************************/
/* InitCounter                                                                     */
/***********************************************************************************/
static void InitCounter()
{
    LARGE_INTEGER li;
    if (!QueryPerformanceFrequency(&li))
    {
        std::cout << "QueryPerformanceFrequency failed!\n";
    }
    PCFreq = double(li.QuadPart) / 1000.0f;
    _qpcInited = true;
}
/***********************************************************************************/
/* End InitCounter                                                                 */
/***********************************************************************************/
/***********************************************************************************/
/* Clock                                                                           */
/***********************************************************************************/
static double CLOCK()
{
    if (!_qpcInited) InitCounter();
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart) / PCFreq;
}
/***********************************************************************************/
/* End Clock                                                                       */
/***********************************************************************************/
/***********************************************************************************/
/* Avgdur                                                                          */
/***********************************************************************************/
static double avgdur(double newdur)
{
    _avgdur = 0.98 * _avgdur + 0.02 * newdur;
    return _avgdur;
}
/***********************************************************************************/
/* End Avgdur                                                                      */
/***********************************************************************************/