#ifndef __TINY_XML_H__ //所有的源文件只能包含一次！！！
#define __TINY_XML_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define T_MALLOC  malloc
#define T_REALLOC realloc
#define T_FREE    free
#define T_STRDUP  strdup

/*
char * __strdup(const char *_string)  //strdup的实现(备用)
{
    size_t len;
    void *new_s;
    if (!_string)
        return NULL;
    len = strlen(_string) + 1;
    new_s = T_MALLOC(len);
    if (new_s == NULL)
        return NULL;
    return (char *)memcpy(new_s, _string, len);
}
 */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

//
// Definitions
//

typedef struct _XMLAttribute
{
	char *key;     //键   //#free!
	char *value;   //值   //#free!
}XMLAttribute;

typedef struct _XMLAttributeList
{
	int attrCapacity;       //键值对(属性) 的当前存储容量
	int attrSize;           //键值对(属性) 存储个数   (获取数量时使用这个)
	XMLAttribute *attrArray;   //键值对(属性) 数组   //#free!
}XMLAttributeList;

void XMLAttributeList_init(XMLAttributeList *attrlist);
void XMLAttributeList_add(XMLAttributeList *attrlist, XMLAttribute *attr);
void XMLAttributeList_free(XMLAttributeList *attrlist);
void XMLAttribute_free(XMLAttribute *attr);

typedef struct _XMLNodeList  //需要前置定义，因为XMLNode中使用了此结构体
{
	int nodeCapacity;   //结点(元素) 存储容量
	int nodeSize;       //结点(元素) 存储个数
	struct _XMLNode **nodeArray;  //存储子元素(地址)的指针数组  #free!
}XMLNodeList;

typedef struct _XMLNode
{
	char *tagName;                 //标签(元素)名称
	char *inner_text;              //标签文本内容
	struct _XMLNode *parent;       //父元素指针
	XMLAttributeList attributes;   //标签属性
	XMLNodeList children;          //子元素列表
}XMLNode;

XMLNode *XMLNode_new(XMLNode *parent);
XMLNode *XMLNode_child(XMLNode *parent, int index);
XMLNodeList *XMLNode_children(XMLNode *parent, const char *tag);
char *XMLNode_attr_val(XMLNode *node, char *key);
XMLAttribute *XMLNode_attr(XMLNode *node, char *key);
void XMLNode_free(XMLNode *node);      //#inner_static

void XMLNodeList_init(XMLNodeList *nodeList);
void XMLNodeList_add(XMLNodeList *nodeList, XMLNode *node);
XMLNode *XMLNodeList_at(XMLNodeList *nodeList, int index);
void XMLNodeList_free(XMLNodeList *nodeList);  //#inner_static
void XMLNodeList_outFree(XMLNodeList *nodeList);

typedef struct _XMLDocument
{
	XMLNode *rootNode;
	char *version;
	char *encoding;
}XMLDoc;

int XMLDocument_load(XMLDoc *doc, const char *path);
int XMLDocument_write(XMLDoc *doc, const char *path, int indent);
void XMLDocument_free(XMLDoc *doc);

//
//  Implementation
//

// @brief: 判断字符串 haystack 是否以 needle 子字符串结尾
static int ends_with(const char *haystack, const char *needle)
{
	int i = 0;
	int h_len = strlen(haystack);
	int n_len = strlen(needle);

	if (h_len < n_len)
		return FALSE;

	for (i = 0; i < n_len; i++)
	{
		if (haystack[h_len - n_len + i] != needle[i])
			return FALSE;
	}

	return TRUE;
}

// @brief: 初始化属性列表，容量置为0
void XMLAttributeList_init(XMLAttributeList *attrlist)
{
	attrlist->attrCapacity = 0;   //全部初始化为0，节约内存
	attrlist->attrSize = 0;
	//list->heap_size = 1;
	//list->size = 0;
	//list->data = (XMLAttribute *)malloc(sizeof(XMLAttribute) * list->heap_size);
}

/*
 * @brief: 把 attr 中存放的属性地址添加到 list 列表中
 * @note: 如果 list->attrSize !=0 ，用完后需要手动释放list->attrArray  //#free!
 */
void XMLAttributeList_add(XMLAttributeList *attrlist, XMLAttribute *attr)
{
	while (attrlist->attrSize >= attrlist->attrCapacity)   //需要重新申请 键值对 空间以存放更多 键值对
	{
		attrlist->attrCapacity += 1; //节约内存      //origin: attrlist->heap_size *= 2;  //2倍空间申请法
		if (attrlist->attrCapacity == 1) //第一次使用
			attrlist->attrArray = (XMLAttribute *)T_MALLOC(sizeof(XMLAttribute) * attrlist->attrCapacity);
		else
			attrlist->attrArray = (XMLAttribute *)T_REALLOC(attrlist->attrArray, sizeof(XMLAttribute) * attrlist->attrCapacity);
	}
	//list->data[list->size].key = (*attr).key;     //直接指向了val和key字符串
	//list->data[list->size].value = (*attr).value;
	//list->size++;
	attrlist->attrArray[attrlist->attrSize++] = *attr;  //相同类型结构体可直接赋值
}

void XMLAttributeList_free(XMLAttributeList *attrlist)
{
	int i;
	for (i = 0; i < attrlist->attrSize; i++)
		XMLAttribute_free(&attrlist->attrArray[i]);  //先释放属性的键值空间
	if (attrlist->attrSize != 0)  //再释放存储属性指针的数组
	{
		T_FREE(attrlist->attrArray);
		attrlist->attrArray = NULL;
	}
	attrlist->attrCapacity = 0;
	attrlist->attrSize = 0;
}

// @brief: 释放 属性(键值对)指向的字符串空间
void XMLAttribute_free(XMLAttribute *attr)
{
	T_FREE(attr->key);
	attr->key = NULL;
	T_FREE(attr->value);
	attr->value = NULL;
}

/*
* @brief: 新建一个子元素，子元素指向父元素，新子元素的各项值初始化为NULL(除parent)
* @return: XMLNode *   (malloc)    //#free!
*/
XMLNode *XMLNode_new(XMLNode *parent)
{
	XMLNode *node = (XMLNode *)T_MALLOC(sizeof(XMLNode));
	node->parent = parent;
	node->tagName = NULL;
	node->inner_text = NULL;
	XMLAttributeList_init(&node->attributes);
	XMLNodeList_init(&node->children);
	if (parent)
		XMLNodeList_add(&parent->children, node);  //把新申请的node地址存放至父元素中的children的指针数组中
	return node;
}

// 返回传入结点的 第index个 子节点指针
XMLNode *XMLNode_child(XMLNode *parent, int index)
{
	return parent->children.nodeArray[index];
}

/*
* @brief: 从传入结点的子一级中查找标签名称为tag的所有元素，并封装成 XMLNodeList
* @return: XMLNodeList * (malloc)   //#free!
*/
XMLNodeList *XMLNode_children(XMLNode *parent, const char *tag)
{
	XMLNodeList *list = (XMLNodeList *)T_MALLOC(sizeof(XMLNodeList));
	XMLNodeList_init(list);

	for (int i = 0; i < parent->children.nodeSize; i++)
	{
		XMLNode *child = parent->children.nodeArray[i];
		if (!strcmp(child->tagName, tag))
			XMLNodeList_add(list, child);
	}

	return list;
}

/*
* @brief: 根据key返回value
* @return: char *  字符串指针  NULL:无
*/
char *XMLNode_attr_val(XMLNode *node, char *key)
{
	for (int i = 0; i < node->attributes.attrSize; i++)
	{
		XMLAttribute attr = node->attributes.attrArray[i];
		if (!strcmp(attr.key, key))
			return attr.value;
	}
	return NULL;
}

//在一个元素的属性列表中根据key返回对应的 XMLAttribute *
XMLAttribute *XMLNode_attr(XMLNode *node, char *key)
{
	for (int i = 0; i < node->attributes.attrSize; i++)
	{
		XMLAttribute *attr = &(node->attributes.attrArray[i]);
		if (!strcmp(attr->key, key))
			return attr;
	}
	return NULL;
}


// @brief: 释放本结点及子节点所占空间
static void XMLNode_free(XMLNode *node)
{
	if (node == NULL)
		return ;
	int i = 0;
	if (node->tagName)
	{
		T_FREE(node->tagName);
		node->tagName = NULL;
	}
	if (node->inner_text)
	{
		T_FREE(node->inner_text);
		node->inner_text = NULL;
	}
	if (node->attributes.attrSize != 0)
		XMLAttributeList_free(&node->attributes);
	if (node->children.nodeSize != 0)
		XMLNodeList_free(&node->children); // #交叉递归释放子元素
	T_FREE(node);
	node = NULL;
}

// @brief: 初始化元素列表，容量置为0
void XMLNodeList_init(XMLNodeList *nodeList)
{
	nodeList->nodeCapacity = 0;   //全部初始化为0，节约内存
	nodeList->nodeSize = 0;
	//nodeList->heap_size = 1;
	//nodeList->size = 0;
	//nodeList->data = (XMLNode **)malloc(sizeof(XMLNode *) * nodeList->heap_size);
}

/*
 * @brief: 把node的地址添加到list的 nodeArray 指针数组中 
 * @note: list->nodeArray 指向的指针数组由malloc得来  //#free!
 */
void XMLNodeList_add(XMLNodeList *nodeList, XMLNode *node)
{
	while (nodeList->nodeSize >= nodeList->nodeCapacity)
	{
		nodeList->nodeCapacity += 1;    //origin:list->heap_size *= 2;   节约内存
		//申请list->node_size个指针大小的指针数组，用于指向上层malloc的node地址
		if (nodeList->nodeCapacity == 1)  //第一次使用
			nodeList->nodeArray = (XMLNode **)T_MALLOC(sizeof(XMLNode *) * nodeList->nodeCapacity);
		else
			nodeList->nodeArray = (XMLNode **)T_REALLOC(nodeList->nodeArray, sizeof(XMLNode *) * nodeList->nodeCapacity);
	}

	nodeList->nodeArray[nodeList->nodeSize++] = node;  //存储子元素地址(指向子元素数据)
}

// @brief: 返回传入结点列表的 第index个 结点指针
XMLNode *XMLNodeList_at(XMLNodeList *nodeList, int index)
{
	return nodeList->nodeArray[index];
}

// @brief: 释放结点列表
static void XMLNodeList_free(XMLNodeList *nodeList)
{
	int i;
	for (i = 0; i < nodeList->nodeSize; i++)
	{
		XMLNode_free(nodeList->nodeArray[i]); // #交叉递归释放子元素
		nodeList->nodeArray[i] = NULL;
	}
	if (nodeList->nodeSize != 0)  //释放存储子元素指针的数组
	{
		T_FREE(nodeList->nodeArray);
		nodeList->nodeArray = NULL;
	}
	nodeList->nodeCapacity = 0;
	nodeList->nodeSize = 0;
}

// @brief: 用于外部 释放结点列表
void XMLNodeList_outFree(XMLNodeList *nodeList)
{
	int i;
	for (i = 0; i < nodeList->nodeSize; i++)
	{
		nodeList->nodeArray[i] = NULL;
	}
	if (nodeList->nodeSize != 0)  //释放存储子元素指针的数组
	{
		T_FREE(nodeList->nodeArray);
		nodeList->nodeArray = NULL;
	}
	nodeList->nodeCapacity = 0;
	nodeList->nodeSize = 0;
	T_FREE(nodeList);

}

enum _TagType
{
	TAG_START,
	TAG_INLINE
};
typedef enum _TagType TagType;

/*
 * @brief: 解析本节点内的数据
 *  @param int * i : buf数组指针位置 '<' 的后一个数据
 *  @param char * lex : 解析数据存储数组（256）
 *  @param int * lexi : 存储数据索引值
 */
static TagType parse_attrs(char *buf, int *i, char *lex, int *lexi, XMLNode *curr_node)
{
	XMLAttribute curr_attr = { NULL, NULL };
	while (buf[*i] != '>')
	{
		lex[(*lexi)++] = buf[(*i)++];

		// Tag name
		if (buf[*i] == ' ' && !curr_node->tagName)
		{
			lex[*lexi] = '\0';
			curr_node->tagName = T_STRDUP(lex);
			*lexi = 0;
			(*i)++;
			continue;
		}

		// Usually ignore spaces
		if (lex[*lexi - 1] == ' ')
		{
			(*lexi)--;  //消除空格占用
		}

		// Attribute key
		if (buf[*i] == '=')
		{
			lex[*lexi] = '\0';
			curr_attr.key = T_STRDUP(lex);
			*lexi = 0;
			continue;
		}

		// Attribute value
		if (buf[*i] == '"')
		{
			if (!curr_attr.key)
			{
				printf("Error: \"%s\" at line %d in %s\r\n", "Value has no key", __LINE__, __FILE__);
				// fprintf(stderr, "Value has no key\n");
				return TAG_START;
			}

			*lexi = 0;
			(*i)++;

			while (buf[*i] != '"')
				lex[(*lexi)++] = buf[(*i)++];
			lex[*lexi] = '\0';
			curr_attr.value = T_STRDUP(lex);
			XMLAttributeList_add(&curr_node->attributes, &curr_attr);
			curr_attr.key = NULL;
			curr_attr.value = NULL;
			*lexi = 0;
			(*i)++;
			continue;
		}

		// Inline node
		if (buf[*i - 1] == '/' && buf[*i] == '>')
		{
			printf("Error: \"%s\" at line %d in %s\r\n", "Unknown Behaviour", __LINE__, __FILE__);
			lex[*lexi] = '\0';
			if (!curr_node->tagName)
			{
				curr_node->tagName = T_STRDUP(lex);
			}
			(*i)++;
			return TAG_INLINE;
		}
	}

	return TAG_START;
}

int XMLDocument_load(XMLDoc *doc, const char *path)
{
	FILE *file = fopen(path, "r");
	if (!file)
	{
		printf("Error: Could not load file from '%s' at line %d in %s\r\n", path, __LINE__, __FILE__);
		// fprintf(stderr, "Could not load file from '%s'\n", path);
		return FALSE;
	}

	fseek(file, 0, SEEK_END); //把文件指针移动到文件结尾
	int size = ftell(file);	  //获取文件大小（字节）
	fseek(file, 0, SEEK_SET); //移动到文件开头

	char *docBuffer = (char *)T_MALLOC(sizeof(char) * size + 1);
	fread(docBuffer, 1, size, file); //把 文件数据加载到堆区
	fclose(file);
	docBuffer[size] = '\0'; //文件结尾符

	doc->rootNode = XMLNode_new(NULL);

	char storeArray[600]; //部分文本内容的存储介质
	int sAi = 0; //storeArray 的索引值
	int dBi = 0; //docBuffer 的索引值

	XMLNode *curr_node = doc->rootNode;

	while (docBuffer[dBi] != '\0') //遍历文档内全部内容
	{
		if (docBuffer[dBi] == '<')
		{
			storeArray[sAi] = '\0';

			// Inner text
			if (sAi > 0)
			{
				if (!curr_node)
				{
					printf("Error: \"%s\" at line %d in %s\r\n", "Text outside of document", __LINE__, __FILE__);
					// fprintf(stderr, "Text outside of document\n");
					return FALSE;
				}
				if (docBuffer[dBi + 1] == '/' && !(curr_node->children.nodeCapacity)) 
				{
					//printf("___%s___\n", lex);
					//当确定是 '<' 最小单元的结束标签，且没有子元素才保存文本内容
					curr_node->inner_text = T_STRDUP(storeArray); // strdup()：maolloc()与参数s字符串相同的空间，然后将s字符串的内容复制到该地址，然后把地址返回
				}
				sAi = 0;
			}

			// End of node
			if (docBuffer[dBi + 1] == '/')
			{
				dBi += 2;  //跳到标签名称
				while (docBuffer[dBi] != '>')
					storeArray[sAi++] = docBuffer[dBi++];
				storeArray[sAi] = '\0';

				if (!curr_node)
				{
					printf("Error: \"%s\" at line %d in %s\r\n", "Already at the root", __LINE__, __FILE__);
					// fprintf(stderr, "Already at the root\n");
					return FALSE;
				}

				if (strcmp(curr_node->tagName, storeArray))
				{
					printf("Error: Mismatched tags (%s != %s) at line %d in %s\r\n", curr_node->tagName, storeArray, __LINE__, __FILE__);
					// fprintf(stderr, "Mismatched tags (%s != %s)\n", curr_node->tag, lex);
					return FALSE;
				}

				curr_node = curr_node->parent;  //切换到父元素
				// Reset lexer
				sAi = 0;
				dBi++;
				continue;
			}

			// Special nodes
			if (docBuffer[dBi + 1] == '!')
			{
				while (docBuffer[dBi] != ' ' && docBuffer[dBi] != '>')
					storeArray[sAi++] = docBuffer[dBi++];
				storeArray[sAi] = '\0';

				// Comments
				if (!strcmp(storeArray, "<!--"))
				{
					storeArray[sAi] = '\0';
					while (!ends_with(storeArray, "-->"))
					{
						storeArray[sAi++] = docBuffer[dBi++];
						storeArray[sAi] = '\0';
					}
					continue;
				}
			}

			// declaration tags
			if (docBuffer[dBi + 1] == '?')
			{
				while (docBuffer[dBi] != ' ' && docBuffer[dBi] != '>')
					storeArray[sAi++] = docBuffer[dBi++];
				storeArray[sAi] = '\0';

				// This is the XML declaration
				if (!strcmp(storeArray, "<?xml"))
				{
					sAi = 0;
					XMLNode *declare = XMLNode_new(NULL);
					parse_attrs(docBuffer, &dBi, storeArray, &sAi, declare);

					XMLAttribute *temp_ver, *temp_enc;
					temp_ver = XMLNode_attr(declare, "version");
					temp_enc = XMLNode_attr(declare, "encoding");
					doc->version = temp_ver->value;   //value指针传递
					doc->encoding = temp_enc->value;
					T_FREE(temp_ver->key);   //key值不再使用，释放掉
					T_FREE(temp_enc->key);
					T_FREE(declare->attributes.attrArray); //把申请的数组释放
					T_FREE(declare);    //释放这个临时结点
					//doc->version = XMLNode_attr_val(declare, "version");
					//doc->encoding = XMLNode_attr_val(declare, "encoding");
					continue;
				}
			}

			// Set current node
			curr_node = XMLNode_new(curr_node);  //新建一个子元素，子元素指向父元素

			// Start tag //开始标签
			dBi++;
			if (parse_attrs(docBuffer, &dBi, storeArray, &sAi, curr_node) == TAG_INLINE)
			{
				printf("Error: \"%s\" at line %d in %s\r\n", "Unknown Behaviour", __LINE__, __FILE__);
				curr_node = curr_node->parent;
				dBi++;
				continue;
			}

			// Set tag name if none
			storeArray[sAi] = '\0';
			if (!curr_node->tagName)
				curr_node->tagName = T_STRDUP(storeArray);

			// Reset lexer
			sAi = 0;
			dBi++;
			continue;
		}
		else
		{
			storeArray[sAi++] = docBuffer[dBi++];
		}
	}
	T_FREE(docBuffer);
	return TRUE;
}

/*
 * @brief: 输出XML文件
 * @param: indent  缩进个数(空格)
 * @param: times   整体缩进 4*times 个空格
 */
static void node_out(FILE *file, XMLNode *node, int indent, int times)
{
	for (int i = 0; i < node->children.nodeSize; i++)
	{
		XMLNode *child = node->children.nodeArray[i];

		if (times > 0) //"%0*s" *是占位符, 表示具体的宽度由后面的参数来指定, printf("%0*s", 5, "123") = printf("%05s", "123")  "00123"
			fprintf(file, "% *s", indent * times, " "); 

		fprintf(file, "<%s", child->tagName);
		for (int i = 0; i < child->attributes.attrSize; i++)  //属性的键值对 个数
		{
			XMLAttribute attr = child->attributes.attrArray[i];
			if (!attr.value || !strcmp(attr.value, "")) //没有属性
				continue;
			fprintf(file, " %s=\"%s\"", attr.key, attr.value);
		}

		if (child->children.nodeSize == 0 && !child->inner_text)
			fprintf(file, "></%s>\n", child->tagName); //无子元素，且无文本内容，直接结束
		else
		{
			fprintf(file, ">");
			if (child->children.nodeSize == 0) //无子元素，直接结束
				fprintf(file, "%s</%s>\n", child->inner_text, child->tagName);
			else
			{
				fprintf(file, "\n");
				node_out(file, child, indent, times + 1);  //进入子元素递归
				//子元素递归完毕，补全结束标签
				if (times > 0)
					fprintf(file, "% *s", indent * times, " ");
				fprintf(file, "</%s>\n", child->tagName);
			}
		}
	}
}

/*
* @brief: 输出XML文件
* @param: indent: 缩进空格数
* @return: FALSE：failed  TRUE：sucessed
*/
int XMLDocument_write(XMLDoc *doc, const char *path, int indent)
{
	FILE *file = fopen(path, "w");
	if (!file)
	{
		printf("Error: Could not open file '%s' at line %d in %s\r\n", path, __LINE__, __FILE__);
		// fprintf(stderr, "Could not open file '%s'\n", path);
		return FALSE;
	}

	fprintf(
		file, "<?xml version=\"%s\" encoding=\"%s\" ?>\n",
		(doc->version) ? doc->version : "1.0",
		(doc->encoding) ? doc->encoding : "UTF-8");
	node_out(file, doc->rootNode, indent, 0);
	fclose(file);
	return TRUE;
}

void XMLDocument_free(XMLDoc *doc)
{
	XMLNode_free(doc->rootNode);
	T_FREE(doc->encoding);  //释放value的值，key的值在解析时已经释放
	doc->encoding = NULL;
	T_FREE(doc->version);
	doc->version = NULL;
}

#endif // __TINY_XML_H__
