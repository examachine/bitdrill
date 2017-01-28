#include "StdAfx.h"
#include "Item.h"
#include <stdio.h>

CItem::CItem(ITEM_NAME s)
{
	name = s;
	/*
	itoa(name,s_name, 10);
	s_nameLen = strlen(s_name);
	s_name[s_nameLen++] = ' ';
	s_name[s_nameLen] = '\0';
	*/
	sprintf(s_name,"%d ", name);
	s_nameLen = strlen(s_name);
	count = 0;
	transactionBitMask = new V_BIT_ARRAY();
}

CItem::~CItem(void)
{
	delete transactionBitMask;
}

int CItem::CompareTo(CItem *o)
{
		// Reverse sort
	return o->count - count;
}
