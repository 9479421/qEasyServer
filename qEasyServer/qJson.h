#pragma once

#include<unordered_map>
#include<map>
#include<vector>
#include<string>
#include<iostream>
#include<sstream>


template<typename T1, typename T2>
class obj
{
public:
	int type; // 0 str     1 digital   2  jsonobject  3 jsonarray      -1考虑搞成Null后面再说
	std::string string;
	double digital;
	T1 jsonObject;
	T2 jsonArray;

public:
	obj(int type, std::string string, double digital, T1 jsonObject, T2 jsonArray) {
		this->type = type;
		this->string = string;
		this->digital = digital;
		this->jsonObject = jsonObject;
		this->jsonArray = jsonArray;
	}



	std::string JsonArrayToString() {
		std::stringstream ssm;
		ssm << "[";
		for (int i = 0; i < jsonArray->maps_obj.size(); i++) {

			int type = jsonArray->maps_obj[i].type;
			if (type == 0)
			{
				ssm << "\"" << jsonArray->maps_obj[i].string << "\"";
			}
			if (type == 1)
			{
				double digital = jsonArray->maps_obj[i].digital;
				int integer = static_cast<int>(digital);
				if (digital == integer)
				{
					ssm << integer;
				}
				else {
					ssm << digital;
				}

			}
			if (type == 2)
			{
				ssm << jsonArray->maps_obj[i].jsonObject.toString();
			}
			if (type == 3)
			{
				ssm << jsonArray->maps_obj[i].jsonArray.toString();
			}

			if (i != jsonArray->maps_obj.size() - 1)
			{
				ssm << ",";
			}
		}
		ssm << "]";


		return ssm.str();
	}
private:

};


class keyVal
{
public:
	keyVal(std::string key ,std::string val) {
		this->key = key;
		this->val = val;
	}
public:
	std::string key;
	std::string val;
};


class qJsonArray;

class qJsonObject
{
public:
	qJsonObject() {

	}
	std::string getString(std::string key) {
		for (std::map<std::string, obj<qJsonObject, qJsonArray*>>::iterator it = maps_obj.begin(); it != maps_obj.end(); ++it) {
			if (key.compare(it->first) == 0)
			{
				return ((obj<qJsonObject, qJsonArray*>)(it->second)).string;
			}
		}
		return "";
	}
	void setString(std::string key, std::string value) {
		maps_obj.insert(std::make_pair(key, obj<qJsonObject, qJsonArray*>(0, value, -1, qJsonObject(), NULL)));
	}
	qJsonObject getJsonObject(std::string key) {
		for (std::map<std::string, obj<qJsonObject, qJsonArray*>>::iterator it = maps_obj.begin(); it != maps_obj.end(); ++it) {
			if (key.compare(it->first) == 0)
			{
				return ((obj<qJsonObject, qJsonArray*>)(it->second)).jsonObject;
			}
		}
		return qJsonObject();
	}
	void setJsonObject(std::string key, qJsonObject value) {
		maps_obj.insert(std::make_pair(key, obj<qJsonObject, qJsonArray*>(2, "", -1, value, NULL)));
	}
	int getInt(std::string key) {

		for (std::map<std::string, obj<qJsonObject, qJsonArray*>>::iterator it = maps_obj.begin(); it != maps_obj.end(); ++it) {
			if (key.compare(it->first) == 0)
			{
				return ((obj<qJsonObject, qJsonArray*>)(it->second)).digital;
			}
		}
		return -1;
	}
	void setInt(std::string key, int value) {
		maps_obj.insert(std::make_pair(key, obj<qJsonObject, qJsonArray*>(1, "", value, qJsonObject(), NULL)));
	}
	double getDouble(std::string key) {
		for (std::map<std::string, obj<qJsonObject, qJsonArray*>>::iterator it = maps_obj.begin(); it != maps_obj.end(); ++it) {
			if (key.compare(it->first) == 0)
			{
				return ((obj<qJsonObject, qJsonArray*>)(it->second)).digital;
			}
		}
		return -1;
	}
	void setDouble(std::string key, double value) {
		maps_obj.insert(std::make_pair(key, obj<qJsonObject, qJsonArray*>(1, "", value, qJsonObject(), NULL)));
	}


	float getFloat(std::string key) {
		for (std::map<std::string, obj<qJsonObject, qJsonArray*>>::iterator it = maps_obj.begin(); it != maps_obj.end(); ++it) {
			if (key.compare(it->first) == 0)
			{
				return ((obj<qJsonObject, qJsonArray*>)(it->second)).digital;
			}
		}
		return -1;
	}
	void setFloat(std::string key, float value) {
		maps_obj.insert(std::make_pair(key, obj<qJsonObject, qJsonArray*>(1, "", value, qJsonObject(), NULL)));
	}

	void setJsonArray(std::string key, qJsonArray* value) {
		maps_obj.insert(std::make_pair(key, obj<qJsonObject, qJsonArray*>(3, "", -1, qJsonObject(), value)));
	}
	qJsonArray* getJsonArray(std::string key) {
		for (std::map<std::string, obj<qJsonObject, qJsonArray*>>::iterator it = maps_obj.begin(); it != maps_obj.end(); ++it) {
			if (key.compare(it->first) == 0)
			{
				return ((obj<qJsonObject, qJsonArray*>)(it->second)).jsonArray;
			}
		}
		return NULL;
	}


	std::string toString() {
		std::stringstream ssm;
		ssm << "{";
		for (std::map<std::string, obj<qJsonObject, qJsonArray*>>::iterator it = maps_obj.begin(); it != maps_obj.end(); ) {
			ssm << "\"" << it->first << "\":";

			int type = ((obj<qJsonObject, qJsonArray*>)(it->second)).type;
			if (type == 0)
			{
				ssm << "\"" << ((obj<qJsonObject, qJsonArray*>)(it->second)).string << "\"";
			}
			if (type == 1)
			{
				double digital = ((obj<qJsonObject, qJsonArray*>)(it->second)).digital;
				int integer = static_cast<int>(digital);
				if (digital == integer)
				{
					ssm << integer;
				}
				else {
					ssm << digital;
				}

			}
			if (type == 2)
			{
				ssm << ((obj<qJsonObject, qJsonArray*>)(it->second)).jsonObject.toString();
			}
			if (type == 3)
			{
			/*	qJsonArray* a = ((obj<qJsonObject, qJsonArray*>)(it->second)).jsonArray;*/
				//printf("%s",it->second.JsonArrayToString().c_str());
				ssm << it->second.JsonArrayToString();
			}

			if (++it != maps_obj.end())
			{
				ssm << ",";
			}
		}

		ssm << "}";

		return ssm.str();
	}

	std::vector<keyVal> getAllKeyVal() {
		std::vector<keyVal> retVector;
		for (std::map<std::string, obj<qJsonObject, qJsonArray*>>::iterator it = maps_obj.begin(); it != maps_obj.end(); it++)
		{
			if (it->second.type == 0)
			{
				retVector.push_back(keyVal(it->first, it->second.string));
			}else if (it->second.type == 1)
			{
				retVector.push_back(keyVal(it->first, std::to_string(it->second.digital)));
			}
			
		}
		return retVector;
	}
private:
	std::map<std::string, obj<qJsonObject, qJsonArray*>> maps_obj;
};


class qJsonArray
{
public:
	qJsonArray() {

	}
	void addJsonObject(qJsonObject jsonObject) {
		maps_obj.push_back(obj<qJsonObject, qJsonArray>(2, "", -1, jsonObject, qJsonArray()));
	}
	qJsonObject getJsonObject(int idx) {
		return maps_obj[idx].jsonObject;
	}
	std::string getString(int idx) {
		return maps_obj[idx].string;
	}
	int getInt(int idx) {
		return maps_obj[idx].digital;
	}
	double getDouble(int idx) {
		return maps_obj[idx].digital;
	}
	double getFloat(int idx) {
		return maps_obj[idx].digital;
	}
	int size() {
		return maps_obj.size();
	}

	void addString(std::string str) {
		maps_obj.push_back(obj<qJsonObject, qJsonArray>(0, str, -1, qJsonObject(), qJsonArray())); //置入字符串
	}
	void addDigital(double value) {
		maps_obj.push_back(obj<qJsonObject, qJsonArray>(1, "", value, qJsonObject(), qJsonArray())); //置入字符串
	}

	std::string toString() {
		std::stringstream ssm;
		ssm << "[";
		for (int i = 0; i < maps_obj.size(); i++) {

			int type = maps_obj[i].type;
			if (type == 0)
			{
				ssm << "\"" << maps_obj[i].string << "\"";
			}
			if (type == 1)
			{
				double digital = maps_obj[i].digital;
				int integer = static_cast<int>(digital);
				if (digital == integer)
				{
					ssm << integer;
				}
				else {
					ssm << digital;
				}

			}
			if (type == 2)
			{
				ssm << maps_obj[i].jsonObject.toString();
			}
			if (type == 3)
			{
				ssm << maps_obj[i].jsonArray.toString();
			}

			if (i != maps_obj.size() - 1)
			{
				ssm << ",";
			}
		}
		ssm << "]";


		return ssm.str();
	}

public:
	std::vector <obj<qJsonObject, qJsonArray>> maps_obj;
};






class qJson
{
public:
	static qJsonObject parseJsonObject(std::string str);
	static qJsonArray* parseJsonArray(std::string str);



};
