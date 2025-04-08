#include <iostream>

#include "AuthDb.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iterator>
#include <unordered_map>

extern std::unordered_map<uint32_t, string> mobileid_user_map;
extern std::unordered_map<string, uint32_t> user_unsecfd;

AuthDb::AuthDb(const char* db_file) :
    _db_file(db_file)
{
}

bool AuthDb::retrieve_cred(string _key, char* cred) {
    DBT key, data;
    char DBRecord[2048];

    /* Zero out the DBTs before using them. */
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    key.data = (void*)_key.c_str();
    key.size = (u_int32_t)(_key.size() + 1);
    data.data = DBRecord;
    data.ulen = sizeof(DBRecord);
    data.flags = DB_DBT_USERMEM;
    if (dbp->get(dbp, NULL, &key, &data, 0) != DB_NOTFOUND) {       
        strncpy_s(cred, BUFFER_SIZE, (char*)data.data, BUFFER_SIZE);
        return true;
    }
    else {
        return false;
    }
}
bool AuthDb::create_db() {
    u_int32_t flags; /* database open flags */
    int ret; /* function return value */

    ret = db_create(&dbp, NULL, 0);
    if (ret != 0) {
        /* Error handling goes here */
        printf("DB UserID Create Error\n");
        return false;
    }
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
        printf("DB userID Open Error\n");
        return false;
    }
    return true;
}

void AuthDb::load_mobile_id() {
    DBC* cursorp;
    DBT key, data;
    int ret;

    dbp->cursor(dbp, NULL, &cursorp, 0);

    memset(&data, 0, sizeof(DBT));
    memset(&key, 0, sizeof(DBT));
    
    while ((ret = cursorp->get(cursorp, &key, &data, DB_NEXT)) == 0) {
        string cred((char*)data.data);
        string id, pw, mobileId;
        istringstream is(cred);
        vector<string> v((istream_iterator<string>(is)), istream_iterator<string>());
        id = v[0];
        pw = v[1];
        mobileId = v[2];
        // TODO: verify string mobileID is an integer.
        mobileid_user_map[stoi(mobileId)] = id;
        printf("[DB] userID:%s, password:%s, mobileId=%s\n", id.c_str(), pw.c_str(), mobileId.c_str());
    }
}
