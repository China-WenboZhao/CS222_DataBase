#include "rm.h"
    


RC RelationManager::PrepareCatalogDescriptor(string tablename,vector<Attribute> &attributes){
	string tables="Tables";
	string columns="Columns";
	Attribute attr;

	if(tables.compare(tablename)==0){
		attr.name="table-id";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=1;
		attributes.push_back(attr);

		attr.name="table-name";
		attr.type=TypeVarChar;
		attr.length=50;
		attr.position=2;
		attributes.push_back(attr);

		attr.name="file-name";
		attr.type=TypeVarChar;
		attr.length=50;
		attr.position=3;
		attributes.push_back(attr);

		attr.name="SystemTable";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=4;
		attributes.push_back(attr);


		return 0;
	}
	else if(columns.compare(tablename)==0){
		attr.name="table-id";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=1;
		attributes.push_back(attr);

		attr.name="column-name";
		attr.type=TypeVarChar;
		attr.length=50;
		attr.position=2;
		attributes.push_back(attr);

		attr.name="column-type";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=3;
		attributes.push_back(attr);

		attr.name="column-length";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=4;
		attributes.push_back(attr);

		attr.name="column-position";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=5;
		attributes.push_back(attr);

		attr.name="NullFlag";
		attr.type=TypeInt;
		attr.length=4;
		attr.position=6;
		attributes.push_back(attr);


		return 0;
	}
	else{
		dprintf("Error! PrepareCatalogDescriptor can only be used to get Catalog's record descriptor\n");
		return -1;
	}

}

RC RelationManager::CreateTablesRecord(void *data,int tableid,const string tablename,int systemtable){
	int offset=0;
	int size = tablename.size();
	char nullind=0;
	
	//copy null indicator
	memcpy((char *)data+offset,&nullind,1);
	offset=offset+1;

	memcpy((char *)data+offset,&tableid,sizeof(int));
	offset=offset+sizeof(int);
	//copy table name
	memcpy((char *)data+offset,&size,sizeof(int));
	offset=offset+sizeof(int);

	memcpy((char *)data+offset,tablename.c_str(),size);
	offset=offset+size;

	//copy file name
	memcpy((char *)data+offset,&size,sizeof(int));
	offset=offset+sizeof(int);

	memcpy((char *)data+offset,tablename.c_str(),size);
	offset=offset+size;

	//copyt SystemTable
	memcpy((char *)data+offset,&systemtable,sizeof(int));
	offset=offset+sizeof(int);
	
	dprintf("\ncreate table record offset is %d\n",offset);

	return 0;

}

RC RelationManager::CreateColumnsRecord(void * data,int tableid, Attribute attr, int position, int nullflag){
	int offset=0;
	int size=attr.name.size();
	char null[1];
	null[0]=0;


	//null indicator
	memcpy((char *)data+offset,null,1);
	offset+=1;

	memcpy((char *)data+offset,&tableid,sizeof(int));
	offset=offset+sizeof(int);

	//copy VarChar
	memcpy((char *)data+offset,&size,sizeof(int));
	offset=offset+sizeof(int);
	memcpy((char *)data+offset,attr.name.c_str(),size);
	offset=offset+size;

	//copy  type
	memcpy((char *)data+offset,&(attr.type),sizeof(int));
	offset=offset+sizeof(int);

	//copy attribute length
	memcpy((char *)data+offset,&(attr.length),sizeof(int));
	offset=offset+sizeof(int);

	//copy position
	memcpy((char *)data+offset,&position,sizeof(int));
	offset=offset+sizeof(int);

	//copy nullflag
	memcpy((char *)data+offset,&nullflag,sizeof(int));
	offset=offset+sizeof(int);
	
	dprintf("\ncreate column record offset is %d\n",offset);

	return 0;

}
RC RelationManager::UpdateColumns(int tableid,vector<Attribute> attributes){
	int size=attributes.size();
	RecordBasedFileManager *rbfm=RecordBasedFileManager::instance();
	FileHandle table_filehandle;
	char *data=(char *)malloc(PAGE_SIZE);
	vector<Attribute> columndescriptor;
	RID rid;

	PrepareCatalogDescriptor("Columns",columndescriptor);
	if(rbfm->openFile("Columns", table_filehandle)==0){

		for(int i=0;i<size;i++){
			CreateColumnsRecord(data,tableid,attributes[i],attributes[i].position,0);
			rbfm->insertRecord(table_filehandle,columndescriptor,data,rid);
			
			dprintf("In UpdateColumns\n");
			rbfm->printRecord(columndescriptor,data);
		}
		rbfm->closeFile(table_filehandle);
		free(data);
		return 0;
	}

	dprintf("There is bug on UpdateColumns\n");
	free(data);
	return -1;
}

int RelationManager::GetFreeTableid(){

	RM_ScanIterator rm_ScanIterator;
	RID rid;
	char *data=(char *)malloc(PAGE_SIZE);

	vector<string> attrname;
	attrname.push_back("table-id");
	int tableID = -1;
	int foundID;
	bool scanID[TABLE_SIZE];
	std::fill_n(scanID,TABLE_SIZE,0);

	void *v = malloc(1);
	if( scan("Tables","",NO_OP,v,attrname,rm_ScanIterator)==0 ){

		while(rm_ScanIterator.getNextTuple(rid,data)!=RM_EOF){
			//!!!! skip null indicator
			memcpy(&foundID,(char *)data+1,sizeof(int));
			dprintf("found table ID is %d\n",foundID);
			scanID[foundID-1]=true;

		}
		for(int i=0;i<TABLE_SIZE;i++){
			if(!scanID[i]){
				tableID=i+1;
				break;
			}
		}

		free(data);
		rm_ScanIterator.close();
		dprintf("Get free table id: %d\n",tableID);
		free(v);
		return tableID;
	}

	dprintf("There is bug on GetFreeTableid\n");
	return -1;

}
RC RelationManager::CreateVarChar(void *data,const string &str){
	int size=str.size();
	int offset=0;
	memcpy((char *)data+offset,&size,sizeof(int));
	offset+=sizeof(int);
	memcpy((char *)data+offset,str.c_str(),size);
	offset+=size;


	return 0;
}

int RelationManager::VarCharToString(void *data,string &str){
	int size;
	int offset=0;
	char * VarCharData=(char *) malloc(PAGE_SIZE);

	memcpy(&size,(char *)data+offset,sizeof(int));
	offset+=sizeof(int);

	memcpy(VarCharData,(char *)data+offset,size);
	offset+=size;

	VarCharData[size]='\0';
	string tempstring(VarCharData);
	str=tempstring;


	free(VarCharData);

	return 0;


}

RelationManager* RelationManager::_rm = 0;

RelationManager* RelationManager::instance()
{
    if(!_rm)
        _rm = new RelationManager();

    return _rm;
}

RelationManager::RelationManager()
{  
//    debug = true;
    rbfm = RecordBasedFileManager::instance();
}

RelationManager::~RelationManager()
{
}

RC RelationManager::createCatalog()
{
	vector<Attribute> tablesdescriptor;
	vector<Attribute> columnsdescriptor;

	RecordBasedFileManager *rbfm=RecordBasedFileManager::instance();
	FileHandle table_filehandle;
	RID rid;


	//creat Tables
	if((rbfm->createFile("Tables"))==0){

		void *data=malloc(PAGE_SIZE);
		int tableid=1;
		int systemtable=1;

		// open table file 
		rbfm->openFile("Tables",table_filehandle);
		
		PrepareCatalogDescriptor("Tables",tablesdescriptor);
		CreateTablesRecord(data,tableid,"Tables",systemtable);
		RC rc = rbfm->insertRecord(table_filehandle,tablesdescriptor,data,rid);
		assert( rc == 0 && "insert table should not fail");
		
		tableid=2;
		CreateTablesRecord(data,tableid,"Columns",systemtable);
		rc = rbfm->insertRecord(table_filehandle,tablesdescriptor,data,rid);
		assert( rc == 0 && "insert table should not fail");
		// close table file
		rbfm->closeFile(table_filehandle);


		//create Columns
		if((rbfm->createFile("Columns"))==0){
			UpdateColumns(1,tablesdescriptor);
			PrepareCatalogDescriptor("Columns",columnsdescriptor);
			UpdateColumns(tableid,columnsdescriptor);

			free(data);
			dprintf("successfully create catalog\n");
			return 0;
		}
	}

	dprintf("Fail to create catalog\n");
	return -1;
}

RC RelationManager::deleteCatalog()
{

	if(rbfm->destroyFile("Tables")==0){
		if(rbfm->destroyFile("Columns")==0){
			dprintf("successfully delete Tables and Columns \n"); 
			return 0;
		}
	}
    return -1;
}

RC RelationManager::createTable(const string &tableName, const vector<Attribute> &attrs)
{
	RecordBasedFileManager *rbfm=RecordBasedFileManager::instance();
	FileHandle filehandle;
	FileHandle nullhandle;
	vector<Attribute> tablesdescriptor;
	char *data=(char *)malloc(PAGE_SIZE);
	RID rid;
	int tableid;
	vector<Attribute> tempattrs=attrs;
	for(int i=0;i< tempattrs.size();i++){
		tempattrs[i].position=i+1;
	}
	if(rbfm->createFile(tableName)==0){


		if(rbfm->openFile("Tables",filehandle)==0){
			dprintf("before get free table id \n");
			tableid=GetFreeTableid();
			dprintf("table id is %d\n",tableid);
			
			PrepareCatalogDescriptor("Tables",tablesdescriptor);
			CreateTablesRecord(data,tableid,tableName,0);
			RC rc = rbfm->insertRecord(filehandle,tablesdescriptor,data,rid);
			assert( rc == 0 && "insert table should not fail");
			
			dprintf("In createTable\n");
			rbfm->printRecord(tablesdescriptor,data);
			    
			rbfm->closeFile(filehandle);
			if(UpdateColumns(tableid,tempattrs)==0){
				free(data);
				return 0;
			}
		}

	}
	assert( false && "There is bug on createTable \n");
	free(data);
	return -1;
}

int RelationManager::getTableId(const string &tableName){

	RM_ScanIterator rm_ScanIterator;
	RID rid;
	int tableid = -1;
	char *VarChardata=(char *)malloc(PAGE_SIZE);
	char *data=(char *)malloc(PAGE_SIZE);
	vector<string> attrname;
	attrname.push_back("table-id");
	int count=0;

	CreateVarChar(VarChardata,tableName);

	dprintf("In getTableId tableName: %s\n",tableName.c_str());

	if( scan("Tables","table-name",EQ_OP,VarChardata,attrname,rm_ScanIterator) == 0 ){
		while(rm_ScanIterator.getNextTuple(rid,data)!=RM_EOF){

			//!!!! skip null indicator
			memcpy(&tableid,(char *)data+1,sizeof(int));
			count++;
			assert( count < 2 && "should be exact one table-id match one table name" );
			if(count>=2){
				cout<<"There are two record in Tables with same table name "<<endl;
			}
		}
		rm_ScanIterator.close();

		free(VarChardata);
		free(data);
		return tableid;
	}
	free(VarChardata);
	free(data);
	return -1;
}

RC RelationManager::deleteTable(const string &tableName)
{
	FileHandle filehandle;
	RM_ScanIterator rm_ScanIterator;
	RM_ScanIterator rm_ScanIterator2;
	RID rid;
	int tableid;

	char *data=(char *)malloc(PAGE_SIZE);
	vector<string> attrname;
	attrname.push_back("table-id");
	vector<RID> rids;

	vector<Attribute> tablesdescriptor;
	PrepareCatalogDescriptor("Tables",tablesdescriptor);
	vector<Attribute> columnsdescriptor;
	PrepareCatalogDescriptor("Columns",columnsdescriptor);
	if(tableName.compare("Tables") && tableName.compare("Columns")){
		if(rbfm->destroyFile(tableName)==0){
			tableid=getTableId(tableName);

			dprintf("\n\nDelete table id %d\n",tableid);

			rbfm->openFile("Tables",filehandle);

			if(RelationManager::scan("Tables","table-id",EQ_OP,&tableid,attrname,rm_ScanIterator)==0){
				while(rm_ScanIterator.getNextTuple(rid,data)!=RM_EOF){
					rids.push_back(rid);
				}
				for(int j=0;j<rids.size();j++){
					rbfm->deleteRecord(filehandle,tablesdescriptor,rids[j]);

				}
				rbfm->closeFile(filehandle);
				rm_ScanIterator.close();

				rbfm->openFile("Columns",filehandle);
				if( scan("Columns","table-id",EQ_OP,&tableid,attrname,rm_ScanIterator2) == 0 ){
					while(rm_ScanIterator2.getNextTuple(rid,data)!=RM_EOF){
						rbfm->deleteRecord(filehandle,columnsdescriptor,rid);
					}
					rm_ScanIterator2.close();
					rbfm->closeFile(filehandle);

					free(data);
					dprintf("Successfully delete %s\n",tableName.c_str());
					return 0;

				}


			}

		}
	}
	assert( false &&"There is bug on deleteTable ");
	free(data);
    return -1;
}

RC RelationManager::getAttributes(const string &tableName, vector<Attribute> &attrs)
{

	RM_ScanIterator rm_ScanIterator;
	RID rid;
	int tableid;
	char *data=(char *)malloc(PAGE_SIZE);
	vector<string> attrname;
	attrname.push_back("column-name");
	attrname.push_back("column-type");
	attrname.push_back("column-length");
	attrname.push_back("column-position");
	attrname.push_back("NullFlag");
	Attribute attr;
	string tempstr;
	int nullflag;
	int offset = 0;



	tableid=getTableId(tableName);  // scan table ID 
	dprintf("tableid ========= %d\n",tableid);
	assert( tableid != -1 && "Table id shouldn't be -1 , maybe forgot to createCatalog first? ");
	if( tableid == -1 ) return -1;

	if( scan("Columns","table-id",EQ_OP,&tableid,attrname,rm_ScanIterator) == 0 ){

		while(rm_ScanIterator.getNextTuple(rid,data)!=RM_EOF){
			//skip null indicatior
			offset=1;
			VarCharToString(data+offset,tempstr);
			attr.name=tempstr;
			offset+=(sizeof(int)+tempstr.size());
			memcpy(&(attr.type),data+offset,sizeof(int));
			offset+=sizeof(int);
			memcpy(&(attr.length),data+offset,sizeof(int));
			offset+=sizeof(int);
			memcpy(&(attr.position),data+offset,sizeof(int));
			offset+=sizeof(int);
			memcpy(&(nullflag),data+offset,sizeof(int));
			offset+=sizeof(int);
			if(nullflag==1){
				attr.length=0;
			}
			attrs.push_back(attr);

		}
		rm_ScanIterator.close();
		free(data);

		dprintf("Successfully getAttribute\nSize of attrs %d\n",attrs.size());
		dprintf("the first attriubt name: %s\nthe last attriubt name: %s\n",attrs[0].name.c_str(),attrs[attrs.size()-1].name.c_str());
		return 0;
	}

	assert( false && "There is bug on getAttribute \n");
	free(data);
	return -1;

}
int RelationManager::IsSystemTable(const string &tableName){
	RM_ScanIterator rm_ScanIterator;
	RID rid;
	int systemtable;
	char *VarChardata=(char *)malloc(PAGE_SIZE);
	char *data=(char *)malloc(PAGE_SIZE);
	vector<string> attrname;
	attrname.push_back("SystemTable");
	int count=0;

	CreateVarChar(VarChardata,tableName);

	if( scan("Tables","table-name",EQ_OP,VarChardata,attrname,rm_ScanIterator) == 0 ){
		while(rm_ScanIterator.getNextTuple(rid,data)!=RM_EOF){
			//!!!! skip null indicator
			memcpy(&systemtable,(char *)data+1,sizeof(int));
			count++;

			assert( count < 2 && "There are two record in Tables with same table name");
			if(count>=2){
				cout<<"There are two record in Tables with same table name "<<endl;
			}
		}
		rm_ScanIterator.close();
		free(VarChardata);
		free(data);
		return systemtable;

	}

	free(VarChardata);
	free(data);
	return -1;
}

RC RelationManager::insertTuple(const string &tableName, const void *data, RID &rid)
{
	FileHandle filehandle;
	vector<Attribute> descriptor;

	if(IsSystemTable(tableName)==0){
		RelationManager::getAttributes(tableName,descriptor);
		if(rbfm->openFile(tableName,filehandle)==0){
			if(rbfm->insertRecord(filehandle,descriptor,data,rid)==0){
				
				dprintf("Successfully insert tuple \n\n\n");

			}
			rbfm->closeFile(filehandle);
			return 0;
		}


	}
	assert(false && "There is bug on insertTuple \n");
	return -1;
}

RC RelationManager::deleteTuple(const string &tableName, const RID &rid)
{
	FileHandle filehandle;
	vector<Attribute> descriptor;

	if(IsSystemTable(tableName)==0){
		RelationManager::getAttributes(tableName,descriptor);
		if(rbfm->openFile(tableName,filehandle)==0){
			if(rbfm->deleteRecord(filehandle,descriptor,rid)==0){
				dprintf("Successfully delete tuple\n");
				rbfm->closeFile(filehandle);
				return 0;
			}
		}
	 }

	assert( false && "There is bug on delete Tuple ");
	return -1;

}

RC RelationManager::updateTuple(const string &tableName, const void *data, const RID &rid)
{
	FileHandle filehandle;
	vector<Attribute> descriptor;

	if(IsSystemTable(tableName)==0){
		getAttributes(tableName,descriptor);
		if(rbfm->openFile(tableName,filehandle)==0){
			if(rbfm->updateRecord(filehandle,descriptor,data,rid)==0){
				dprintf("Successfully update tuple \n");
				RC rc = rbfm->closeFile(filehandle);
				assert( rc == 0 && "close file not successful");
				return 0;
			}
		}
	}
	assert(false && "There is bug on update Tuple \n");
	return -1;
}

RC RelationManager::readTuple(const string &tableName, const RID &rid, void *data)
{
	FileHandle filehandle;
	vector<Attribute> descriptor;
	getAttributes(tableName,descriptor);
	if(rbfm->openFile(tableName,filehandle)==0){
		if(rbfm->readRecord(filehandle,descriptor,rid,data)==0){
			dprintf("Successfully read tuple \n");
			RC rc = rbfm->closeFile(filehandle);
			assert( rc == 0 && "something wrong in rbfm->closeFile ");
			return 0;
		}
	}

	dprintf("There is bug on read Tuple \n");
	return -1;
}

RC RelationManager::printTuple(const vector<Attribute> &attrs, const void *data)
{
	if(rbfm->printRecord(attrs,data)==0){
		dprintf("Successfully print tuple\n");
		return 0;
	}

	dprintf("There is bug on print Tuple \n");
	return -1;
}

RC RelationManager::readAttribute(const string &tableName, const RID &rid, const string &attributeName, void *data)
{
	FileHandle filehandle;
	vector<Attribute> descriptor;

	getAttributes(tableName,descriptor);
	if(rbfm->openFile(tableName,filehandle)==0){
		if(rbfm->readAttribute(filehandle,descriptor,rid,attributeName,data)==0){
			dprintf("Successfully read attribute \n");
			rbfm->closeFile(filehandle);
			return 0;
		}
	}

	dprintf("There is bug on read attribute \n");
	return -1;
}

RC RelationManager::scan(const string &tableName,
      const string &conditionAttribute,
      const CompOp compOp,                  
      const void *value,                    
      const vector<string> &attributeNames,
      RM_ScanIterator &rm_ScanIterator)
{
	FileHandle filehandle;
	vector<Attribute> descriptor;
	if(tableName.compare("Tables")==0){
		PrepareCatalogDescriptor("Tables",descriptor);
		dprintf("In scan prepareCatalogDescriptor(Tables,descriptor)\n%s\n%s\n",descriptor[0].name.c_str(),(descriptor.back()).name.c_str()); 
	}
	else if(tableName.compare("Columns")==0){
		PrepareCatalogDescriptor("Columns",descriptor);
		dprintf("In scan prepareCatalogDescriptor(Columns,descriptor)\n%s\n%s\n",descriptor[0].name.c_str(),(descriptor.back()).name.c_str()); 
	}
	else{
		RelationManager::getAttributes(tableName,descriptor);
	}
	if(rbfm->openFile(tableName,filehandle)==0){
		if(rbfm->scan(filehandle,descriptor,conditionAttribute,compOp,value,attributeNames,rm_ScanIterator.rbfm_ScanIterator)==0){
			dprintf("Successfully doing RelationManager scan \n");
			return 0;
		}
	}
	dprintf("There is bug on doing RelationManager scan \n");
	return -1;
}

// Extra credit work
RC RelationManager::addAttribute(const string &tableName, const Attribute &attr)
{
	vector<Attribute> descriptor;
	int position;
	int tableid;
	vector<Attribute> tempattr;
	tempattr.push_back(attr);


	tableid=getTableId(tableName);
	RelationManager::getAttributes(tableName,descriptor);
	position=(descriptor.back()).position +1;
	tempattr[0].position=position;
	if(UpdateColumns(tableid,tempattr)==0){

		dprintf("Successfully doing addAttribute \n");
		return 0;
	}
	dprintf("There is bug on  addAttribute \n");
	return -1;
}

// Extra credit work
RC RelationManager::dropAttribute(const string &tableName, const string &attributeName)
{
	RecordBasedFileManager *rbfm=RecordBasedFileManager::instance();
	FileHandle filehandle;
	vector<Attribute> descriptor;
	RM_ScanIterator rm_ScanIterator;
	RID rid;
	int tableid;
	char *data=(char *)malloc(PAGE_SIZE);
	vector<string> attrname;
	attrname.push_back("table-id");
	attrname.push_back("column-name");
	attrname.push_back("column-type");
	attrname.push_back("column-length");
	attrname.push_back("column-position");
	attrname.push_back("NullFlag");
	char *VarChardata=(char *) malloc(PAGE_SIZE);

	string tempstr;
	int nullflag;
	int offset = 0;


	if(tableName.compare("Tables") && tableName.compare("Columns")){
		tableid=getTableId(tableName);
		if(RelationManager::scan("Columns","table-id",EQ_OP,&tableid,attrname,rm_ScanIterator)==0){
			while(rm_ScanIterator.getNextTuple(rid,data)!=RM_EOF){
				//skip null indicator and tableid
				memcpy(VarChardata,(char *)data+1+sizeof(int),50);
				VarCharToString(VarChardata,tempstr);
				if(tempstr.compare(attributeName)==0){
					//skip null indicatior
					nullflag=1;
					offset=1+2*sizeof(int)+tempstr.size()+3*sizeof(int);
					memcpy((char *)data+offset,&nullflag,sizeof(int));
					rbfm->openFile("Columns",filehandle);
					PrepareCatalogDescriptor("Columns",descriptor);
					if(rbfm->updateRecord(filehandle,descriptor,data,rid)==0){
						rm_ScanIterator.close();
						free(data);
						free(VarChardata);
						dprintf("Successfully dropAttribute \n");
						rbfm->closeFile(filehandle);
						return 0;
					}
				}
			}

		}
	}
	dprintf("There is bug on dropAttribute \n");
	free(data);
	free(VarChardata);
	return -1;
}
RC RelationManager::printTable(const string &tableName){
	RM_ScanIterator rm_ScanIterator;
	RID rid;
	char *data=(char *)malloc(PAGE_SIZE);
	vector<Attribute> recordDescriptor;
	getAttributes(tableName,recordDescriptor);
	vector<string> attrname;
	for(int j=0;j<recordDescriptor.size();j++){
		attrname.push_back(recordDescriptor[j].name);
	}

	if( scan(tableName,"",NO_OP,NULL,attrname,rm_ScanIterator)==0 ){
		while(rm_ScanIterator.getNextTuple(rid,data)!=RM_EOF){
			//!!!! skip null indicator
			rbfm->printRecord(recordDescriptor,data);

		}
		free(data);
		rm_ScanIterator.close();
		dprintf("Successfully printTabale \n");
		return 0;

	}
	dprintf("There is bug on printTabale \n");
	free(data);
	return -1;
}
