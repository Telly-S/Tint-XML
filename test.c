#include "txml.h"

int main(void)
{
	XMLDoc doc;
	int i;
	if (XMLDocument_load(&doc, "../test.xml"))
	{
		// doc.rootNode 只有 XMLNodeList children 可用
		XMLNode* str = XMLNode_child(doc.rootNode, 0);  // 获取根节点
		
		XMLNodeList* shapes = XMLNode_children(str, "shape");  // 从根节点下获取标签为 shape 的所有node
		for (i = 0; i < shapes->nodeSize; i++)
		{
			XMLNode* shape = XMLNodeList_at(shapes, i);
			XMLAttribute* type = XMLNode_attr(shape, "type");
			printf("%s = %s\n", type->key, type->value);  // 打印出 type 的键值对
		}

		XMLNode* parentShape = XMLNodeList_at(shapes, 0);
		printf("parentShape's child is %s, has %d nodes\n", parentShape->children.nodeArray[0]->tagName, parentShape->children.nodeSize);
		XMLNode* points = parentShape->children.nodeArray[0];
		for (i = 0; i < points->children.nodeSize; i++)
		{
			printf("%s ", points->children.nodeArray[i]->inner_text);
		}
		//XMLDocument_write(&doc, "out.xml", 4);  // 输出一个xml文件

		XMLNodeList_outFree(shapes);  // 释放占用的内存
		XMLDocument_free(&doc);       // 释放doc内存
	}

	getchar();
	return 0;
}
