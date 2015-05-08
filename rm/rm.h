
#ifndef _rm_h_
#define _rm_h_

#include <string>
#include <vector>
#include <climits>
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <bitset>

#include "../rbf/rbfm.h"

using namespace std;
# define TABLE_SIZE 4096
# define RM_EOF (-1)  // end of a scan operator

// RM_ScanIterator is an iteratr to go through tuples
class RM_ScanIterator : public DebugMsg {
public:
  RM_ScanIterator() {};
  ~RM_ScanIterator() {};
  RBFM_ScanIterator rbfm_ScanIterator;

  // "data" follows the same format as RelationManager::insertTuple()
  RC getNextTuple(RID &rid, void *data) { return rbfm_ScanIterator.getNextRecord(rid,data); };
  RC close() { return rbfm_ScanIterator.close(); };
};


// Relation Manager
class RelationManager : public DebugMsg
{
public:
  static RelationManager* instance();

  RC createCatalog();

  RC deleteCatalog();

  RC createTable(const string &tableName, const vector<Attribute> &attrs);

  RC deleteTable(const string &tableName);

  RC getAttributes(const string &tableName, vector<Attribute> &attrs);

  RC insertTuple(const string &tableName, const void *data, RID &rid);

  RC deleteTuple(const string &tableName, const RID &rid);

  RC updateTuple(const string &tableName, const void *data, const RID &rid);

  RC readTuple(const string &tableName, const RID &rid, void *data);

  // mainly for debugging
  // Print a tuple that is passed to this utility method.
  RC printTuple(const vector<Attribute> &attrs, const void *data);

  // mainly for debugging
  RC readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data);

  // scan returns an iterator to allow the caller to go through the results one by one.
  RC scan(const string &tableName,
      const string &conditionAttribute,
      const CompOp compOp,                  // comparison type such as "<" and "="
      const void *value,                    // used in the comparison
      const vector<string> &attributeNames, // a list of projected attributes
      RM_ScanIterator &rm_ScanIterator);
  RC printTable(const string &tableName);

// Extra credit work (10 points)
public:
  RC dropAttribute(const string &tableName, const string &attributeName);

  RC addAttribute(const string &tableName, const Attribute &attr);


protected:
  RelationManager();
  ~RelationManager();

private:
  static RelationManager *_rm;

  RecordBasedFileManager *rbfm; 
  int VarCharToString(void *data, string &str);
  int GetFreeTableid();
  int getTableId(const string &tableName);
  int IsSystemTable(const string &tableName);
  RC UpdateColumns(int tableid,vector<Attribute> attributes);
  RC CreateVarChar(void *data,const string &str);



  RC PrepareCatalogDescriptor(string tablename,vector<Attribute> &attributes);
  RC CreateTablesRecord(void *data,int tableid,string tablename,int systemtable);
  RC CreateColumnsRecord(void * data,int tableid, Attribute attr, int position, int nullflag);
 
};

#endif
