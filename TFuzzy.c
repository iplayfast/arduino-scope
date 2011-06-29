#include "TFuzzy.h"

void TFuzzyInit(struct TFuzzy *data)
{
	data->DataLength = 0;
}
int TFuzzyAdd(struct TFuzzy *data,VALUE_TYPE X,VALUE_TYPE Y)
{
int r = data->DataLength;
	data->Data[data->DataLength].x = X;
	data->Data[data->DataLength].y = Y;
	data->DataLength++;
	if (data->DataLength==FUZZY_DATA_LENGTH)
		data->DataLength=0;
	return r;
}
void DeleteItem(struct TFuzzy *data,int i)
{
	for(;i<data->DataLength-1;i++)
	{
		data->Data[i].x = data->Data[i+1].x;
		data->Data[i].y = data->Data[i+1].y;
	}
	data->Data[data->DataLength].x = 0;	// not really needed, just easier when debugging
	data->Data[data->DataLength].y = 0;
	data->DataLength--;
}
void TFuzzyAddFuzzy(struct TFuzzy *data,struct TFuzzyXY f)
{
	TFuzzyAdd(data,f.x,f.y);
}
void TFuzzyAddFuzzyp(struct TFuzzy *data,const struct TFuzzyXY *f)
{
	TFuzzyAdd(data,f->x,f->y);
}

void TFuzzySetValueAt(struct TFuzzy *data,int i,VALUE_TYPE x,VALUE_TYPE y)
{
	data->Data[i].x = x; data->Data[i].y = y;
}

const struct TFuzzyXY *GetItemc(const struct TFuzzy *data,int i)
{
	if (i>=data->DataLength) i = data->DataLength-1;
	if (i<0) i =0;
	return &data->Data[i];
}
struct TFuzzyXY *GetItem(struct TFuzzy *data,int i)
{
	return &data->Data[i];
}

int TFuzzyCount(struct TFuzzy *data)
{
	return data->DataLength;
}

	// adds a point to the fuzzy table, returns index of point added
int TFuzzyAddPoint(struct TFuzzy *data,VALUE_TYPE x,VALUE_TYPE y)
{
int i;
struct TFuzzyXY *t,*n;
	for( i = 0;i< TFuzzyCount(data);i++)
    {
		t = GetItem(data,i);
        if (t->x== x)
        {
    		t->y = (t->y + y) / (2.0);//average original point and new point
			return i;
        };
    };
//    SetSize(Count()+1);
	if (data->DataLength==FUZZY_DATA_LENGTH-1)
		Optimize(data,1.0);	// try to optimized the current numbers;
	if (data->DataLength<FUZZY_DATA_LENGTH-1)
		data->DataLength++;

	for(i = data->DataLength-1,t = GetItem(data,i);;i--)	// insertion sort
	{
		if ((i == 0) || (GetItemc(data,i-1)->x < x))
		{
			t->x = x;
			t->y = y;
			return i;
		}
		else
		{
			n = GetItem(data,i-1);
			t->x = n->x;
			t->y = n->y;
			t = n;
		};
	 };

}

VALUE_TYPE Value(const struct TFuzzy *data,VALUE_TYPE x)
{
int i,l,Previous,Next;
VALUE_TYPE	m;
const struct TFuzzyXY *t;
	l = data->DataLength;
	Previous = 0;
	Next = 0;
	if (l==0) return -1.0;
	for(i = 0;i< l;i++)
	{
		t = GetItemc(data,i);
		if (t->x < x)
			Previous = i;
		if (t->x == x)
        {
    	    Previous = i;
			Next = i;
            break;
        };
		if (t->x > x) break;
        Next = i+1;
    };
    if (Next>l-1) Next = l-1;
    if (Next==Previous)
		return GetItemc(data,Next)->y;
	else
	{
	const struct TFuzzyXY *p,*n;
		p = GetItemc(data,Previous);
		n = GetItemc(data,Next);
                if (p->y==n->y) return p->y;
		  // result = mx + b;
#ifdef USE_INT
                  {
                  VALUE_TYPE y = (((n->y * ( x - p->x)) + (p->y * (n->x - x)))/ (n->x - p->x));
//                  VALUE_TYPE y = (((n->x * (p->x - p->y)) + (x * (p->y - n->y)))/ (p->x - n->x));
//                  VALUE_TYPE b = (((p->x * n->y) - (p->y * n->x))/ (p->x - n->x));
//                  VALUE_TYPE y = (b + (x * (p->y - n->y) / (p->x - n->x)));
                  return y;
                  }
		  m =(n->y - p->y) / (n->x - p->x);
		  return (m * (x - p->x) + p->y);



		  m =(VALUE_SCALE * (n->y - p->y)) / (n->x - p->x);
		  {
		  VALUE_TYPE r = (m * (x - p->x) + p->y)/VALUE_SCALE;
		  if (r<=0)
		  {
			m =(VALUE_SCALE * (p->y - n->y)) / (p->x - n->x);
			r = (m * (x - n->x) + n->y)/VALUE_SCALE;
		  }
		  return r;
		  }
#else
        	{
                  VALUE_TYPE y = (((n->y * ( x - p->x)) + (p->y * (n->x - x)))/ (n->x - p->x));
//                  VALUE_TYPE y = (((n->x * (p->x - p->y)) + (x * (p->y - n->y)))/ (p->x - n->x));
                	return y;
                  }
		  m =(n->y - p->y) / (n->x - p->x);
		  return (m * (x - p->x) + p->y);
#endif
	};
}
VALUE_TYPE MINVT(VALUE_TYPE f,VALUE_TYPE s)
{
  if (f>s) return s; else return f;
}

VALUE_TYPE MAXVT(VALUE_TYPE f,VALUE_TYPE s)
{
  if (f>s) return f; else return s;
}

int TFuzzySetValueXY(struct TFuzzy *data,VALUE_TYPE x,VALUE_TYPE y) // Sets a point to the fuzzy table, returns index of point added
{
int i;
struct TFuzzyXY *t;

	// if already in list then just set it
	for (i = 0;i<data->DataLength;i++)
	{
		t = GetItem(data,i);
		if (t->x > x)
			break;
		if (t->x==x)
		{
			t->y = y;
			return i;
		};
	};
	// not in list so add it
	return TFuzzyAdd(data,x,y);
};

 // returns a y value for the index into the table
VALUE_TYPE ValueAt(const struct TFuzzy *data,unsigned int idx)
{
	return GetItemc(data,idx)->y;
}
 // returns a x value for the index into the table
VALUE_TYPE IndexAt(const struct TFuzzy *data,unsigned int idx )
{
	return GetItemc(data,idx)->x;
}
VALUE_TYPE LowestRange(const struct TFuzzy *data) // returns the lowest x
{
	return GetItemc(data,0)->x;
}
VALUE_TYPE HighestRange(const struct TFuzzy *data)// returns the highest x
{
  return GetItemc(data,data->DataLength-1)->x;
}
void TFuzzySwapXY(struct TFuzzy *data)
{
int i;
  for (i=0;i<data->DataLength;i++)
	TFuzzySetValueAt(data,i,ValueAt(data,i),IndexAt(data,i));
}

VALUE_TYPE TFuzzyEquality(struct TFuzzy *data,struct TFuzzy *t)
{
int i;
VALUE_TYPE	v,v1,result;
	result = 0.0;
	if (t->DataLength<data->DataLength)
	{
		for(i = 0;i<t->DataLength;i++)
		{
			v = Value(data,IndexAt(t,i));
			v1 = ValueAt(t,i);
			if (v==0)  v = 0.00001;	// avoid divide by 0 problems
			if (v1==0)  v1 = 0.00001;
			v = 1 / v;
			v1 = 1 / v1;
			result = result + v - v1;
		};
		if (t->DataLength>1)
			result = result / (t->DataLength-1);
	}
	else
	{
		for(i = 0;i<data->DataLength;i++)
		{
			v1 = Value(t,IndexAt(data,i));
			v = ValueAt(data,i);
			if (v==0)  v = 0.00001;
			if (v1==0)  v1 = 0.00001;
			if (v1<0.00001)  v1 = 0.00001;
			if (v<0.00001)  v = 0.00001;
			v = 1 / v;
			v1 = 1 / v1;
			result = result + v - v1;
		  };
			if (data->DataLength>1)
				result = result / (data->DataLength -1);
	};
	if (result < 0.0)  result = -result;
	result = (1.0 - result) * 100;
	return result;
};

// remove values that would normally fall within the range of factor
void Optimize(struct TFuzzy *data,VALUE_TYPE factor)
{
int i;
VALUE_TYPE	s1,s2,factor2;
struct TFuzzy test;
int change = 1;
	if (FUZZY_DATA_LENGTH<4) return;
        test.DataLength=4;
  while(change)
  {
      change = 0;
      i = 1;
      factor2 = factor * 2;
      while(i<data->DataLength-1)
      {
      	test.Data[0].x = IndexAt(data,i-1);
      	test.Data[0].y = ValueAt(data,i-1);
      	test.Data[1].x = IndexAt(data,i+1);
      	test.Data[1].y = ValueAt(data,i+1);
        s1 = ValueAt(data,i);	// calculated value
        s2 = Value(&test,IndexAt(data,i));	// saved value

            if ((fabs(s1-s2)<=factor))// does it fall within limit?
            {   // yes we can remove it
                    DeleteItem(data,i);
                    change = 1;
            }
            else
            i++;
      };
  };
/*
 struct TFuzzyXY *n,*p;// next,prev
 VALUE_TYPE	va,v;
  i = 1;
  while(i<data->DataLength-1)
  {
	va = ValueAt(data,i) - factor;
	n = GetItem(data,i+1);
	p = GetItem(data,i-1);
	s1 = (n->y - p->y);
	s2 = (n->x - p->x);
	if (s2<0.0001)
		 s2 = 0.0001;
	if (s2!=0)
		v = GetItemc(data,i)->x * s1 / s2;
	else
		v = GetItemc(data,i)->x * s1 / 0.0001;
	v = v - factor;
	if ((s1==0) || ((v<va) && (v+factor2>=va)) || ((v>=va) && (v-factor2<=va)))
	{
		DeleteItem(data,i);
		i = 1;
	}
	else
		i++;
  };
*/
};

// add additional values (of the calculated value to data)
void TFuzzyIncreaseSamples(struct TFuzzy *data)
{
int i;
VALUE_TYPE	x1,x2;
	for(i=0;i<data->DataLength-1;i+=2)
	{
		x1 = IndexAt(data,i);
		x2 = IndexAt(data,i+1);
		x1 = (x1+x2) /2;
		TFuzzySetValueXY(data,x1,Value(data,x1));	// insert calculated value into where it would turn up
	}
}

VALUE_TYPE InhibitValue(const struct TFuzzy *data,VALUE_TYPE In,VALUE_TYPE InhibitPercent)
{
VALUE_TYPE	v1,v2,v3;
int i;
	v1 = 0.0;
	for(i=In-10;i<In;i++)
		v1 += Value(data,i);
	v1 /= 10.0;	// get average of previous 10 (totally arbitrary)

	v2 = Value(data,In);
	v3 = 0.0;
	for(i=In+1;i<In+11;i++)
		v3 += Value(data,i);
	v3 /= 10.0;	// average of next 10

	v1 *= InhibitPercent;
	v3 *= InhibitPercent;
	v2 = v2 - (v1 + v3);
	return v2;
}

void TFuzzySimpleInhibit(struct TFuzzy *data,VALUE_TYPE InhibitPercent)
{
int i;
VALUE_TYPE	v1,v2,v3,v4;
	v1 = ValueAt(data,0);
  v2 = ValueAt(data,1);
  v3 = ValueAt(data,2);
  v4 = ValueAt(data,3);
//  InhibitPercent = 100.0 / InhibitPercent;
  TFuzzySetValueAt(data,0,IndexAt(data,0),0);
  for(i = 1;i<data->DataLength;i++)
  {
	v1 = (v1 * InhibitPercent) + (v3 * InhibitPercent);
	TFuzzySetValueAt(data,i,IndexAt(data,i),v2- v1);
	v1 = v2; v2 = v3; v3 = v4;
	if (i<(data->DataLength-4))
		v4 = ValueAt(data,i+3);
  }
}
void CopyAndInhibit(struct TFuzzy *data,const struct TFuzzy *Fuzzy)
{
struct TFuzzy temp;
	temp.DataLength=0;
	CopyAndInhibitTemp(data,Fuzzy,&temp);
	Optimize(data,1);
}
void CopyAndInhibitTemp(struct TFuzzy *data,const struct TFuzzy *Fuzzy,struct TFuzzy *Temp)
{
VALUE_TYPE l,h,r,d;
	data->DataLength=0;
	l = LowestRange(Fuzzy);
	h = HighestRange(Fuzzy);
	if ((l<0) || (h<0)) return;
	d = (h - l) / (Fuzzy->DataLength * 10);
	r = l;

//    Temp.SetUsedLength(ceil(h - l));	// make space needed
	Temp->DataLength = 0;
	data->DataLength = 0;
	while(r<h)
	{
		TFuzzyAdd(data,r,Value(Fuzzy,r));
		r = r + d;
	};
	r = l + d;
	while(r<(h-d))		// do inhibit from previous and next (if all 3 are equal  value=0)
	{
		TFuzzyAdd(Temp,r,Value(data,r) - (Value(data,r-d)/2.0) - (Value(data,r+d)/2.0));
		r = r + d;
	};
	data->DataLength=0;	// empty now!
	r = l + d;
	TFuzzyAdd(data,r,Value(Temp,r));
	TFuzzyAdd(data,h-d,Value(Temp,h-d));
	while(r<(h-d))
	{
		TFuzzyAdd(data,r,Value(Temp,r-d)-Value(Temp,r) + Value(Temp,r) - Value(Temp,r+d));
		r = r + d;
	};
}

void TFuzzyRange(struct TFuzzy *data,VALUE_TYPE Floor,VALUE_TYPE Ceil)
{
VALUE_TYPE v;
int Size = data->DataLength;
	if (Size==0) return;
	{
	VALUE_TYPE min = GetItemc(data,0)->y;
	VALUE_TYPE max;
	int i;
	struct TFuzzyXY *t;
		// find the minimum value of the y axis
		for(i=1;i<Size;i++)
		{
			t = GetItem(data,i);
			if (t->y<min) min = t->y;
		}
		// remove the minimum value and add the floor
		max= GetItemc(data,0)->y - min + Floor;	// first one to set up max for loop interior
		for(i = 0;i<Size;i++)
		{
			t = GetItem(data,i);
			v = t->y - min + Floor;
			if (v>max) max = v;	// find the max
			t->y = v;
		}
		// max is adjusted to max relative to ceiling
		max = Ceil / max;
		for(i = 0;i<Size;i++)
		{
			t = GetItem(data,i);
			t->y = t->y * max;
		}
	}
}

VALUE_TYPE Or(const struct TFuzzy *data,const struct TFuzzy *f, VALUE_TYPE v)
{
VALUE_TYPE r1,r2;
	r1 = Value(f,v);
	r2 = Value(data,v);
	if (r1>r2) return r1;
	else return r2;
}
VALUE_TYPE OrResult(const struct TFuzzy *data,VALUE_TYPE In,VALUE_TYPE CurrentResult)
{
VALUE_TYPE r2;
	r2 = Value(data,In);
	if (CurrentResult>r2) return CurrentResult;
	else return r2;
}
VALUE_TYPE And(const struct TFuzzy *data,const struct TFuzzy *f, VALUE_TYPE v)
{
VALUE_TYPE r1,r2;
	r1 = Value(f,v);
	r2 = Value(data,v);
	if (r1<r2) return r1;
	else return r2;
}
VALUE_TYPE AndResult(const struct TFuzzy *data,VALUE_TYPE In,VALUE_TYPE CurrentResult)
{
VALUE_TYPE r2;
	r2 = Value(data,In);
	if (CurrentResult<r2) return CurrentResult;
	else return r2;
}

VALUE_TYPE XOr(const struct TFuzzy *data,const struct TFuzzy *f, VALUE_TYPE v)
{
VALUE_TYPE r1,r2;
	r1 = Value(f,v);
	r2 = Value(data,v);
	if (r1<r2) return r2 - r1;
	else return r1 - r2;
}

VALUE_TYPE Not(const struct TFuzzy *data,VALUE_TYPE v,VALUE_TYPE TrueValue)
{
	return TrueValue - Value(data,v);
}



#ifdef VALIDATING
bool TFuzzyTest(bool Verbose,CryObject &Object,bool (CallBack)(bool Verbose,const char *Result,bool fail))
{
	return CryArray::Test(Verbose,Object,CallBack);
}
#endif


