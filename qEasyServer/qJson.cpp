#include "qJson.h"

QJsonObject* QJsonObject::parseObject(char* str)
{
	QJsonObject* retJsonObject = new QJsonObject();

	QJsonChild** jsonChild = &retJsonObject->m_jsonChild;
	if (str[0] == '{' && str[strlen(str) - 1] == '}')
	{
		//去除首尾的括号
		str[strlen(str) - 1] = '\0';
		str++;
		while (true) {
			bool flag = false;
			char* des = NULL;
			//分割,
			char* temp = strchr(str, ',');
			if (temp == NULL)
			{
				//str就等于str本身
				flag = true;
				des = strdup(str);
			}
			else {
				*temp++ = '\0';
				//此时str等于要解析的值
				des = strdup(str);
				str = temp;
			}
			char* val = strpbrk(des, ":");
			QJsonObject* valObj = NULL;
			if (val)
			{
				int type = STRING;
				*val++ = '\0';
				char* key = des;
				//去除key的双引号
				key[strlen(key) - 1] = '\0';
				key++;
				//如果是文本型去除双引号
				if (val[0] == '"' && val[strlen(val) - 1] == '"')
				{
					type = STRING;
					val[strlen(val) - 1] = '\0';
					val++;
					//printf("key: %s val: %s\n", key, val);
				}
				if (val[0] == '{' && val[strlen(val) - 1] == '}')
				{
					type = JSONOBJECT;
					//printf("jsonobject: %s\n", val);
					valObj = QJsonObject::parseObject(val);
					//printf("after jsonobject: %s\n", val);
				}
				if (*jsonChild == NULL) //说明是第一个成员
				{
					*jsonChild = new QJsonChild();
					(*jsonChild)->key = key;
					if (type == STRING)
					{
						(*jsonChild)->type = STRING;
						(*jsonChild)->valStr = val;
					}
					else if (type == JSONOBJECT) {
						(*jsonChild)->type = JSONOBJECT;
						(*jsonChild)->valObj = valObj;
					}
				}
				else {
					QJsonChild* tail = *jsonChild;
					while (tail->next != NULL)
					{
						tail = tail->next;
					}
					tail->next = new QJsonChild();
					tail->next->key = key;
					if (type == STRING)
					{
						tail->next->type = STRING;
						tail->next->valStr = val;
					}
					else if (type == JSONOBJECT) {
						tail->next->type = JSONOBJECT;
						tail->next->valObj = valObj;
					}
				}
			}
			if (flag)
			{
				break;
			}
		}
	}
	//printf("m_jsonChild.key:%s\n", retJsonObject->m_jsonChild->key);
	//printf("m_jsonChild.val:%s\n", retJsonObject->m_jsonChild->valStr);
	return retJsonObject;
}

char* QJsonObject::getString(const char* key)
{
	do
	{
		if (strcmp(key, m_jsonChild->key) == 0)
		{
			return m_jsonChild->valStr;
		}
	} while ((m_jsonChild = m_jsonChild->next) != NULL);

	return NULL;
}

QJsonObject* QJsonObject::getJsonObject(const char* key)
{
	do
	{
		if (strcmp(key, m_jsonChild->key) == 0)
		{
			return m_jsonChild->valObj;
		}
	} while ((m_jsonChild = m_jsonChild->next) != NULL);

	return NULL;
}
