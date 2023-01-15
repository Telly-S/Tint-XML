#ifndef __TINY_XML_H__ //���е�Դ�ļ�ֻ�ܰ���һ�Σ�����
#define __TINY_XML_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define T_MALLOC  malloc
#define T_REALLOC realloc
#define T_FREE    free
#define T_STRDUP  strdup

/*
char * __strdup(const char *_string)  //strdup��ʵ��(����)
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
	char *key;     //��   //#free!
	char *value;   //ֵ   //#free!
}XMLAttribute;

typedef struct _XMLAttributeList
{
	int attrCapacity;       //��ֵ��(����) �ĵ�ǰ�洢����
	int attrSize;           //��ֵ��(����) �洢����   (��ȡ����ʱʹ�����)
	XMLAttribute *attrArray;   //��ֵ��(����) ����   //#free!
}XMLAttributeList;

void XMLAttributeList_init(XMLAttributeList *attrlist);
void XMLAttributeList_add(XMLAttributeList *attrlist, XMLAttribute *attr);
void XMLAttributeList_free(XMLAttributeList *attrlist);
void XMLAttribute_free(XMLAttribute *attr);

typedef struct _XMLNodeList  //��Ҫǰ�ö��壬��ΪXMLNode��ʹ���˴˽ṹ��
{
	int nodeCapacity;   //���(Ԫ��) �洢����
	int nodeSize;       //���(Ԫ��) �洢����
	struct _XMLNode **nodeArray;  //�洢��Ԫ��(��ַ)��ָ������  #free!
}XMLNodeList;

typedef struct _XMLNode
{
	char *tagName;                 //��ǩ(Ԫ��)����
	char *inner_text;              //��ǩ�ı�����
	struct _XMLNode *parent;       //��Ԫ��ָ��
	XMLAttributeList attributes;   //��ǩ����
	XMLNodeList children;          //��Ԫ���б�
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

// @brief: �ж��ַ��� haystack �Ƿ��� needle ���ַ�����β
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

// @brief: ��ʼ�������б�������Ϊ0
void XMLAttributeList_init(XMLAttributeList *attrlist)
{
	attrlist->attrCapacity = 0;   //ȫ����ʼ��Ϊ0����Լ�ڴ�
	attrlist->attrSize = 0;
	//list->heap_size = 1;
	//list->size = 0;
	//list->data = (XMLAttribute *)malloc(sizeof(XMLAttribute) * list->heap_size);
}

/*
 * @brief: �� attr �д�ŵ����Ե�ַ��ӵ� list �б���
 * @note: ��� list->attrSize !=0 ���������Ҫ�ֶ��ͷ�list->attrArray  //#free!
 */
void XMLAttributeList_add(XMLAttributeList *attrlist, XMLAttribute *attr)
{
	while (attrlist->attrSize >= attrlist->attrCapacity)   //��Ҫ�������� ��ֵ�� �ռ��Դ�Ÿ��� ��ֵ��
	{
		attrlist->attrCapacity += 1; //��Լ�ڴ�      //origin: attrlist->heap_size *= 2;  //2���ռ����뷨
		if (attrlist->attrCapacity == 1) //��һ��ʹ��
			attrlist->attrArray = (XMLAttribute *)T_MALLOC(sizeof(XMLAttribute) * attrlist->attrCapacity);
		else
			attrlist->attrArray = (XMLAttribute *)T_REALLOC(attrlist->attrArray, sizeof(XMLAttribute) * attrlist->attrCapacity);
	}
	//list->data[list->size].key = (*attr).key;     //ֱ��ָ����val��key�ַ���
	//list->data[list->size].value = (*attr).value;
	//list->size++;
	attrlist->attrArray[attrlist->attrSize++] = *attr;  //��ͬ���ͽṹ���ֱ�Ӹ�ֵ
}

void XMLAttributeList_free(XMLAttributeList *attrlist)
{
	int i;
	for (i = 0; i < attrlist->attrSize; i++)
		XMLAttribute_free(&attrlist->attrArray[i]);  //���ͷ����Եļ�ֵ�ռ�
	if (attrlist->attrSize != 0)  //���ͷŴ洢����ָ�������
	{
		T_FREE(attrlist->attrArray);
		attrlist->attrArray = NULL;
	}
	attrlist->attrCapacity = 0;
	attrlist->attrSize = 0;
}

// @brief: �ͷ� ����(��ֵ��)ָ����ַ����ռ�
void XMLAttribute_free(XMLAttribute *attr)
{
	T_FREE(attr->key);
	attr->key = NULL;
	T_FREE(attr->value);
	attr->value = NULL;
}

/*
* @brief: �½�һ����Ԫ�أ���Ԫ��ָ��Ԫ�أ�����Ԫ�صĸ���ֵ��ʼ��ΪNULL(��parent)
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
		XMLNodeList_add(&parent->children, node);  //���������node��ַ�������Ԫ���е�children��ָ��������
	return node;
}

// ���ش������ ��index�� �ӽڵ�ָ��
XMLNode *XMLNode_child(XMLNode *parent, int index)
{
	return parent->children.nodeArray[index];
}

/*
* @brief: �Ӵ��������һ���в��ұ�ǩ����Ϊtag������Ԫ�أ�����װ�� XMLNodeList
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
* @brief: ����key����value
* @return: char *  �ַ���ָ��  NULL:��
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

//��һ��Ԫ�ص������б��и���key���ض�Ӧ�� XMLAttribute *
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


// @brief: �ͷű���㼰�ӽڵ���ռ�ռ�
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
		XMLNodeList_free(&node->children); // #����ݹ��ͷ���Ԫ��
	T_FREE(node);
	node = NULL;
}

// @brief: ��ʼ��Ԫ���б�������Ϊ0
void XMLNodeList_init(XMLNodeList *nodeList)
{
	nodeList->nodeCapacity = 0;   //ȫ����ʼ��Ϊ0����Լ�ڴ�
	nodeList->nodeSize = 0;
	//nodeList->heap_size = 1;
	//nodeList->size = 0;
	//nodeList->data = (XMLNode **)malloc(sizeof(XMLNode *) * nodeList->heap_size);
}

/*
 * @brief: ��node�ĵ�ַ��ӵ�list�� nodeArray ָ�������� 
 * @note: list->nodeArray ָ���ָ��������malloc����  //#free!
 */
void XMLNodeList_add(XMLNodeList *nodeList, XMLNode *node)
{
	while (nodeList->nodeSize >= nodeList->nodeCapacity)
	{
		nodeList->nodeCapacity += 1;    //origin:list->heap_size *= 2;   ��Լ�ڴ�
		//����list->node_size��ָ���С��ָ�����飬����ָ���ϲ�malloc��node��ַ
		if (nodeList->nodeCapacity == 1)  //��һ��ʹ��
			nodeList->nodeArray = (XMLNode **)T_MALLOC(sizeof(XMLNode *) * nodeList->nodeCapacity);
		else
			nodeList->nodeArray = (XMLNode **)T_REALLOC(nodeList->nodeArray, sizeof(XMLNode *) * nodeList->nodeCapacity);
	}

	nodeList->nodeArray[nodeList->nodeSize++] = node;  //�洢��Ԫ�ص�ַ(ָ����Ԫ������)
}

// @brief: ���ش������б�� ��index�� ���ָ��
XMLNode *XMLNodeList_at(XMLNodeList *nodeList, int index)
{
	return nodeList->nodeArray[index];
}

// @brief: �ͷŽ���б�
static void XMLNodeList_free(XMLNodeList *nodeList)
{
	int i;
	for (i = 0; i < nodeList->nodeSize; i++)
	{
		XMLNode_free(nodeList->nodeArray[i]); // #����ݹ��ͷ���Ԫ��
		nodeList->nodeArray[i] = NULL;
	}
	if (nodeList->nodeSize != 0)  //�ͷŴ洢��Ԫ��ָ�������
	{
		T_FREE(nodeList->nodeArray);
		nodeList->nodeArray = NULL;
	}
	nodeList->nodeCapacity = 0;
	nodeList->nodeSize = 0;
}

// @brief: �����ⲿ �ͷŽ���б�
void XMLNodeList_outFree(XMLNodeList *nodeList)
{
	int i;
	for (i = 0; i < nodeList->nodeSize; i++)
	{
		nodeList->nodeArray[i] = NULL;
	}
	if (nodeList->nodeSize != 0)  //�ͷŴ洢��Ԫ��ָ�������
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
 * @brief: �������ڵ��ڵ�����
 *  @param int * i : buf����ָ��λ�� '<' �ĺ�һ������
 *  @param char * lex : �������ݴ洢���飨256��
 *  @param int * lexi : �洢��������ֵ
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
			(*lexi)--;  //�����ո�ռ��
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

	fseek(file, 0, SEEK_END); //���ļ�ָ���ƶ����ļ���β
	int size = ftell(file);	  //��ȡ�ļ���С���ֽڣ�
	fseek(file, 0, SEEK_SET); //�ƶ����ļ���ͷ

	char *docBuffer = (char *)T_MALLOC(sizeof(char) * size + 1);
	fread(docBuffer, 1, size, file); //�� �ļ����ݼ��ص�����
	fclose(file);
	docBuffer[size] = '\0'; //�ļ���β��

	doc->rootNode = XMLNode_new(NULL);

	char storeArray[600]; //�����ı����ݵĴ洢����
	int sAi = 0; //storeArray ������ֵ
	int dBi = 0; //docBuffer ������ֵ

	XMLNode *curr_node = doc->rootNode;

	while (docBuffer[dBi] != '\0') //�����ĵ���ȫ������
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
					//��ȷ���� '<' ��С��Ԫ�Ľ�����ǩ����û����Ԫ�زű����ı�����
					curr_node->inner_text = T_STRDUP(storeArray); // strdup()��maolloc()�����s�ַ�����ͬ�Ŀռ䣬Ȼ��s�ַ��������ݸ��Ƶ��õ�ַ��Ȼ��ѵ�ַ����
				}
				sAi = 0;
			}

			// End of node
			if (docBuffer[dBi + 1] == '/')
			{
				dBi += 2;  //������ǩ����
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

				curr_node = curr_node->parent;  //�л�����Ԫ��
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
					doc->version = temp_ver->value;   //valueָ�봫��
					doc->encoding = temp_enc->value;
					T_FREE(temp_ver->key);   //keyֵ����ʹ�ã��ͷŵ�
					T_FREE(temp_enc->key);
					T_FREE(declare->attributes.attrArray); //������������ͷ�
					T_FREE(declare);    //�ͷ������ʱ���
					//doc->version = XMLNode_attr_val(declare, "version");
					//doc->encoding = XMLNode_attr_val(declare, "encoding");
					continue;
				}
			}

			// Set current node
			curr_node = XMLNode_new(curr_node);  //�½�һ����Ԫ�أ���Ԫ��ָ��Ԫ��

			// Start tag //��ʼ��ǩ
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
 * @brief: ���XML�ļ�
 * @param: indent  ��������(�ո�)
 * @param: times   �������� 4*times ���ո�
 */
static void node_out(FILE *file, XMLNode *node, int indent, int times)
{
	for (int i = 0; i < node->children.nodeSize; i++)
	{
		XMLNode *child = node->children.nodeArray[i];

		if (times > 0) //"%0*s" *��ռλ��, ��ʾ����Ŀ���ɺ���Ĳ�����ָ��, printf("%0*s", 5, "123") = printf("%05s", "123")  "00123"
			fprintf(file, "% *s", indent * times, " "); 

		fprintf(file, "<%s", child->tagName);
		for (int i = 0; i < child->attributes.attrSize; i++)  //���Եļ�ֵ�� ����
		{
			XMLAttribute attr = child->attributes.attrArray[i];
			if (!attr.value || !strcmp(attr.value, "")) //û������
				continue;
			fprintf(file, " %s=\"%s\"", attr.key, attr.value);
		}

		if (child->children.nodeSize == 0 && !child->inner_text)
			fprintf(file, "></%s>\n", child->tagName); //����Ԫ�أ������ı����ݣ�ֱ�ӽ���
		else
		{
			fprintf(file, ">");
			if (child->children.nodeSize == 0) //����Ԫ�أ�ֱ�ӽ���
				fprintf(file, "%s</%s>\n", child->inner_text, child->tagName);
			else
			{
				fprintf(file, "\n");
				node_out(file, child, indent, times + 1);  //������Ԫ�صݹ�
				//��Ԫ�صݹ���ϣ���ȫ������ǩ
				if (times > 0)
					fprintf(file, "% *s", indent * times, " ");
				fprintf(file, "</%s>\n", child->tagName);
			}
		}
	}
}

/*
* @brief: ���XML�ļ�
* @param: indent: �����ո���
* @return: FALSE��failed  TRUE��sucessed
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
	T_FREE(doc->encoding);  //�ͷ�value��ֵ��key��ֵ�ڽ���ʱ�Ѿ��ͷ�
	doc->encoding = NULL;
	T_FREE(doc->version);
	doc->version = NULL;
}

#endif // __TINY_XML_H__
