#ifndef CONTAINER_INCLUDED
#define CONTAINER_INCLUDED

class Vert
{
private:
	double *coord;
	bool overhang;
public:
	Vert() {}
	Vert(double x, double y, double z)
	{
		coord = new double[3];
		coord[0] = x;
		coord[1] = y;
		coord[2] = z;
		overhang = false;
	}

	void SetOverHang() { overhang = true; }
	void Transalte(double d) { coord[0] += d; coord[1] += d; coord[2] += d; }
	bool GetOverHang(){return overhang;}
    double *GetCoord(){return coord;}

};

class Face
{
private:
    int *vlist;
    double *norm;
	bool overhang;

public:
    Face(){}
    Face(int v1, int v2, int v3)
    {
        vlist=new int[3];
        vlist[0]=v1;
        vlist[1]=v2;
        vlist[2]=v3;
		overhang = false;
    }

	void SetAsOverHang() { overhang = true; }
	void SetNorm(double *n_norm){norm =n_norm;}
    int *GetAllVert(){return vlist;}
    double *GetNorm(){return norm;}
	bool GetOverHange() { return overhang; }
};


#endif // CONTAINER_INCLUDED
