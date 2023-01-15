#include "txml.h"

int main(void)
{
	XMLDoc doc;
	int i;
	if (XMLDocument_load(&doc, "../test.xml"))
	{
		// doc.rootNode ֻ�� XMLNodeList children ����
		XMLNode* str = XMLNode_child(doc.rootNode, 0);  // ��ȡ���ڵ�
		
		XMLNodeList* shapes = XMLNode_children(str, "shape");  // �Ӹ��ڵ��»�ȡ��ǩΪ shape ������node
		for (i = 0; i < shapes->nodeSize; i++)
		{
			XMLNode* shape = XMLNodeList_at(shapes, i);
			XMLAttribute* type = XMLNode_attr(shape, "type");
			printf("%s = %s\n", type->key, type->value);  // ��ӡ�� type �ļ�ֵ��
		}

		XMLNode* parentShape = XMLNodeList_at(shapes, 0);
		printf("parentShape's child is %s, has %d nodes\n", parentShape->children.nodeArray[0]->tagName, parentShape->children.nodeSize);
		XMLNode* points = parentShape->children.nodeArray[0];
		for (i = 0; i < points->children.nodeSize; i++)
		{
			printf("%s ", points->children.nodeArray[i]->inner_text);
		}
		//XMLDocument_write(&doc, "out.xml", 4);  // ���һ��xml�ļ�

		XMLNodeList_outFree(shapes);  // �ͷ�ռ�õ��ڴ�
		XMLDocument_free(&doc);       // �ͷ�doc�ڴ�
	}

	getchar();
	return 0;
}
