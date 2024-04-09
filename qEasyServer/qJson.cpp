#include "qJson.h"
#include <stdarg.h>
#include<vector>



std::string trim(const std::string& str) {
	// 找到第一个不是空格的字符的位置
	auto start = str.find_first_not_of(' ');
	// 如果字符串全是空格，返回一个空字符串
	if (start == std::string::npos)
		return "";

	// 找到最后一个不是空格的字符的位置
	auto end = str.find_last_not_of(' ');

	// 返回从开始到结束位置的子字符串
	return str.substr(start, end - start + 1);
}

int findFirstNotOf(const std::string& str, char ch...) {
	va_list chList;
	va_start(chList, ch);

	std::vector<char> chVector;


	int nArgVal = 0;
	for (;;)
	{
		nArgVal = va_arg(chList, char);
		if (nArgVal == 0)
		{
			break;
		}
		else {
			chVector.push_back(nArgVal);
		}

	}

	va_end(chList);

	for (int i = 0; i < str.size(); i++)
	{
		bool flag = false;
		for (int j = 0; j < chVector.size(); j++)
		{
			if (chVector[j] == str[i]) //有任何一个相等的，都不能算
			{
				flag = true;
			}
		}
		if (flag == false)
		{
			return i;
		}
	}
	return -1;
}

bool isDigit(std::string str) {
	if (str[0] == '.' || str[str.size() - 1] == '.')
	{
		return false;
	}
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!((str[i] >= '0' && str[i] <= '9') || str[i] == '.')) {
			return false;
		}
	}
	return true;
}


qJsonObject qJson::parseJsonObject(std::string str)
{
	qJsonObject* retObject = new qJsonObject();


	str = trim(str);
	int size = str.size();
	if (str[0] == '{' && str[size - 1] == '}') //判断符合字符串规范
	{
		str = str.substr(1, size - 2); //去除了首尾的括号

		bool end = false;
		while (true)
		{
			if (end) //读到了json字符串结尾
			{
				break;
			}

			////前缀有逗号的情况即使出现 后面也会自动pass掉

			int idx = str.find(":"); //只要key里不要出现:就OK

			std::string front = str.substr(0, idx);
			int yin1 = front.find_first_of('"');
			int yin2 = front.find_last_of('"');
			std::string key = front.substr(yin1 + 1, yin2 - (yin1 + 1));

			std::string after = str.substr(idx + 1);
			int firstCharIdx = after.find_first_not_of(' ');
			char firstChar = after.at(firstCharIdx);
			if (firstChar == '"') //字符串
			{
				for (int i = firstCharIdx + 1; i < after.size(); i++)
				{
					if (after[i] == '"' && after[i - 1] != '\\')  //找到value的结尾了
					{

						std::string value = after.substr(firstCharIdx + 1, i - (firstCharIdx + 1));
						////字符串结尾了
						str = str.substr(idx + 1 + i + 1); //+ firstCharIdx

						//printf("key:%s value:%s\n", key.c_str(), value.c_str());
						//写入
						retObject->setString(key, value);


						if (str.find_first_not_of(' ') == -1) //读到结尾了
						{
							end = true;
						}
						break;
					}
				}
			}
			else if (firstChar == '{') //jsonObject
			{
				//寻找after的结尾;
				int middle = 1, big = 0, shuangyinhao = 0;

				for (int i = firstCharIdx + 1; i < after.size(); i++) {


					if (after[i] == '"' && after[i - 1] != '\\')
					{
						shuangyinhao++;
					}

					if (shuangyinhao % 2 != 1)  //说明现在不在字符串里
					{
						if (after[i] == '{')
						{
							middle++;
						}
						if (after[i] == '}')
						{
							middle--;
						}
						if (after[i] == '[')
						{
							big++;
						}
						if (after[i] == ']')
						{
							big--;
						}
					}
					if (middle == 0 && shuangyinhao % 2 == 0 && big == 0) //说明两边全部都对上了，这个对象结束了
					{

						//获取对象字符串 {*****}
						std::string objStr = after.substr(firstCharIdx, i - firstCharIdx + 1);
						//截断，保留后面的文本
						str = str.substr(idx + 1 + firstCharIdx + i + 1);
						//写入 
						retObject->setJsonObject(key, /*递归*/parseJsonObject(objStr/*对象文本*/)); //同时将该对象传递

						if (str.find_first_not_of(' ') == -1) //读到结尾了
						{
							end = true;
						}
						break;
					}

				}
			}
			else if (firstChar == '[') //jsonObject
			{
				//寻找after的结尾]
				int middle = 0, big = 1, shuangyinhao = 0;
				for (int i = firstCharIdx + 1; i < after.size(); i++) {
					if (after[i] == '"' && after[i - 1] != '\\')
					{
						shuangyinhao++;
					}

					if (shuangyinhao % 2 != 1)  //说明现在不在字符串里
					{
						if (after[i] == '{')
						{
							middle++;
						}
						if (after[i] == '}')
						{
							middle--;
						}
						if (after[i] == '[')
						{
							big++;
						}
						if (after[i] == ']')
						{
							big--;
						}
					}
					if (middle == 0 && shuangyinhao % 2 == 0 && big == 0) //说明两边全部都对上了，这个对象结束了
					{

						//获取数组字符串 [*****]
						std::string arrStr = after.substr(firstCharIdx, i - firstCharIdx + 1);
						//截断，保留后面的文本
						str = str.substr(idx + 1 + firstCharIdx + i + 1);
						//写入 
						retObject->setJsonArray(key, /*递归*/parseJsonArray(arrStr/*对象文本*/)); //同时将该对象传递

						if (str.find_first_not_of(' ') == -1) //读到结尾了
						{
							end = true;
						}
						break;
					}

				}

			}
			else {  //说明是数字或者null
				std::string value;
				int endIdx = after.find(',');
				if (endIdx == -1) //说明是结尾了
				{
					value = trim(after.substr(firstCharIdx));
					endIdx = after.size() - 1; //设置索引到结尾
				}
				else {
					value = trim(after.substr(firstCharIdx, endIdx - firstCharIdx));
				}

				str = str.substr(idx + 1 + endIdx + 1);
				if (value.compare("NULL") == 0 || value.compare("null") == 0) //说明值为null
				{
					//这种情况可以忽略了
					if (str.find_first_not_of(' ') == -1) //读到结尾了
					{
						end = true;
					}




				}
				else {
					//判断是否合法数字，如果不是，说明json不合规
					if (isDigit(value))
					{
						retObject->setDouble(key, std::stod(value));
						if (str.find_first_not_of(' ') == -1) //读到结尾了
						{
							end = true;
						}
					}
					else {
						//json不合规，直接抛出异常
						throw new std::exception(); //"format exception"
					}
				}

			}
		}
	}

	return *retObject;
}

qJsonArray* qJson::parseJsonArray(std::string str)
{
	qJsonArray* retJsonArray = new qJsonArray();


	str = trim(str);
	int size = str.size();
	if (str[0] == '[' && str[size - 1] == ']') //判断符合字符串规范
	{
		str = str.substr(1, size - 2); //去除了首尾的括号


		//数值类型
		bool end = false;
		while (true)
		{
			if (end) //读到了json字符串结尾
			{
				break;
			}

			int idx = findFirstNotOf(str, ' ', ',');  //第一个字的位置
			//判断是字符串类型还是Null还是数字类型
			if (str.at(idx) == '"')
			{
				for (size_t i = idx + 1; i < str.size(); i++)
				{
					if (str[i] == '"' && str[i - 1] != '\\')
					{
						std::string value = str.substr(idx + 1, i - (idx + 1));
						retJsonArray->addString(value);
						str = str.substr(i + 1);

						if (str.find_first_not_of(' ') == -1)
						{
							end = true;
						}
						break;
					}
				}
			}
			else if (str.at(idx) == '{') {
				std::string value;


				int middle = 1, big = 0, shuangyinhao = 0;
				for (size_t i = idx + 1; i < str.size(); i++)
				{
					if (str[i] == '"' && str[i - 1] != '\\')
					{
						shuangyinhao++;
					}

					if (shuangyinhao % 2 != 1)  //说明现在不在字符串里
					{
						if (str[i] == '{')
						{
							middle++;
						}
						if (str[i] == '}')
						{
							middle--;
						}
						if (str[i] == '[')
						{
							big++;
						}
						if (str[i] == ']')
						{
							big--;
						}
					}
					if (middle == 0 && shuangyinhao % 2 == 0 && big == 0) //说明两边全部都对上了，这个对象结束了
					{
						std::string jsonObjectStr = str.substr(idx, i - idx + 1);
						retJsonArray->addJsonObject(parseJsonObject(jsonObjectStr));

						str = str.substr(i + 1);

						if (str.find_first_not_of(' ') == -1)
						{
							end = true;
						}

						break;
					}
				}
			}
			else if (str.at(idx) == '[') {
				//这种情况待添加

			}
			else {
				//为null或者数字
				std::string value;
				int endIdx = str.find(',', idx);
				if (endIdx == -1) //找到字符串结尾了
				{
					endIdx = str.size() - 1;
					value = trim(str.substr(idx, endIdx - idx + 1));
				}
				else {
					value = trim(str.substr(idx, endIdx - idx));
				}

				//读取值
				if (value.compare("NULL") == 0 || value.compare("null") == 0) //说明值为null
				{
					//这种情况可以忽略了
					if (str.find_first_not_of(' ') == -1) //读到结尾了
					{
						end = true;
					}


				}
				else {
					//判断是否合法数字，如果不是，说明json不合规
					if (isDigit(value))
					{
						retJsonArray->addDigital(std::stod(value));
						str = str.substr(endIdx + 1);
						if (str.find_first_not_of(' ') == -1) //读到结尾了
						{
							end = true;
						}
					}
					else {
						//json不合规，直接抛出异常
						throw std::exception(); //"format exception"
					}
				}
			}
		}

	}


	return retJsonArray;
}
