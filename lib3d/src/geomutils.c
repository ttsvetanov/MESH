/* $Id: geomutils.c,v 1.1 2001/03/12 14:50:32 aspert Exp $ */
#include <3dmodel.h>


vertex ncrossp(vertex p1, vertex p2, vertex p3) {
  vertex v1, v2, tmp;
  double norm;

  v1.x = p2.x - p1.x;
  v1.y = p2.y - p1.y;
  v1.z = p2.z - p1.z;

  v2.x = p3.x - p1.x;
  v2.y = p3.y - p1.y;
  v2.z = p3.z - p1.z;

  tmp.x = v1.y*v2.z - v1.z*v2.y;
  tmp.y = v1.z*v2.x - v1.x*v2.z;
  tmp.z = v1.x*v2.y - v1.y*v2.x;
  
  norm = sqrt(tmp.x*tmp.x + tmp.y*tmp.y + tmp.z*tmp.z);
  if (fabs(norm) < 1e-10) {
    printf("ncrossp: Trouble\n");
  }

  tmp.x /= norm;
  tmp.y /= norm;
  tmp.z /= norm;
  
  return tmp;
}

vertex crossprod(vertex v1, vertex v2) {
  vertex res;

  res.x = v1.y*v2.z - v1.z*v2.y;
  res.y = v1.z*v2.x - v1.x*v2.z;
  res.z = v1.x*v2.y - v1.y*v2.x;

  return res;
}

/*Used for 2D triangulation */
double cross_product2d(vertex p1, vertex p2, vertex p3) {
    vertex v1, v2;

    v1.x = p2.x - p1.x;
    v1.y = p2.y - p1.y;

    v2.x = p3.x - p1.x;
    v2.y = p3.y - p1.y;

    return (v1.x*v2.y - v1.y*v2.x);
}

double scalprod(vertex v1, vertex v2) {
  return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

double norm(vertex v) {
  return (sqrt(v.x*v.x + v.y*v.y + v.z*v.z));
}


void normalize(vertex *v) {
  double n;

  n = norm(*v);
  
  if (fabs(n) < 1e-10) /* Do nothing if vector is too small */
    return;
  
  v->x /= n;
  v->y /= n;
  v->z /= n;
}

/* Returns 1 if p is inside the circle defined by center & r */
int inside(vertex p, vertex center, double r) {
    vertex tmp;

    tmp.x = p.x - center.x;
    tmp.y = p.y - center.y;
    tmp.z = p.z - center.z;

    
    if (norm(tmp) < r)
	return 1;
    else 
	return 0;
}


void compute_circle2d(vertex p1, vertex p2, vertex p3, 
		      double *r, vertex *center) {
    vertex v1, v2;
    double cp; 
    double num, np1, np2, np3;


    v1.x = p2.x - p1.x;
    v1.y = p2.y - p1.y;
    
    v2.x = p3.x - p1.x;
    v2.y = p3.y - p1.y;
    
    cp = v1.x*v2.y - v1.y*v2.x;

    if (fabs(cp)>1e-10) {
	np1 = norm(p1);
	np2 = norm(p2);
	np3 = norm(p3);
	num = np1*(p2.y - p3.y) + np2*(p3.y - p1.y) +
	    np3*(p1.y - p2.y);
	center->x = num/(2.0*cp);
	num = np1*(p3.x - p2.x) + np2*(p1.x - p3.x) +
	    np3*(p2.x - p1.x);
	center->y = num/(2.0*cp);
    }

    v1.x = center->x - p1.x;
    v1.y = center->y - p1.y;
    v1.z = 0.0;

    *r = norm(v1);

}

/* Compute the center and radius of the circle passing
   by the points p1,p2 and p3 */
void compute_circle3d(vertex p1, vertex p2, vertex p3, 
		    double *r, vertex *center) {
    vertex u,v,w;
    vertex m1, m2, tmp;
    int i;
    double det;
    double a[9], b[3];

    u.x = p2.x - p1.x;
    u.y = p2.y - p1.y;
    u.z = p2.z - p1.z;

    v.x = p3.x - p1.x;
    v.y = p3.y - p1.y;
    v.z = p3.z - p1.z;

    w = crossprod(u,v);

   
    /* The center of the circle is the 
       intersection of 3 planes */

    m1.x = (p1.x + p2.x)*0.5;
    m1.y = (p1.y + p2.y)*0.5;
    m1.z = (p1.z + p2.z)*0.5;

    m2.x = (p1.x + p3.x)*0.5;
    m2.y = (p1.y + p3.y)*0.5;
    m2.z = (p1.z + p3.z)*0.5;

    /* Is the equation system OK ? */
    det = u.x*(v.y*w.z - v.z*w.y) - 
	v.x*(u.y*w.z - u.z*w.y) + w.x*(u.y*v.z - u.z*v.y);
    
    if (fabs(det) < 1e-6) {
	printf("Error solving system\n");
	exit(0);
    }

    /* Brute force matrix inversion */
    a[0] = (v.y*w.z - v.z*w.y)/det;
    a[1] = (u.z*w.y - u.y*w.z)/det;
    a[2] = (u.y*v.z - u.z*v.y)/det;
    a[3] = (v.z*w.x - v.x*w.z)/det;
    a[4] = (u.x*w.z - u.z*w.x)/det;
    a[5] = (u.z*v.x - v.z*u.x)/det;
    a[6] = (v.x*w.y - w.x*v.y)/det;
    a[7] = (u.y*w.x - w.y*u.x)/det;
    a[8] = (u.x*v.y - v.x*u.y)/det;

    b[0] = u.x*m1.x + u.y*m1.y + u.z*m1.z;
    b[1] = v.x*m2.x + v.y*m2.y + v.z*m2.z;
    b[2] = 0.0;

   
    center->x = 0.0;
    for (i=0; i<3; i++)
	 center->x += a[i]*b[i];
    
    center->y = 0.0;
    for (i=0; i<3; i++)
	 center->y += a[i+3]*b[i];

    center->z = 0.0;
    for (i=0; i<3; i++)
	 center->z += a[i+6]*b[i];

    tmp.x = center->x - p1.x;
    tmp.y = center->y - p1.y;
    tmp.z = center->z - p1.z;

    *r = norm(tmp);
    
}

double tri_area(vertex p1, vertex p2, vertex p3) {
    vertex u,v,h;
    double nu,nv,uv,nh;
    double tmp;

    u.x = p1.x - p3.x;
    u.y = p1.y - p3.y;
    u.z = p1.z - p3.z;
    
    v.x = p2.x - p3.x;
    v.y = p2.y - p3.y;
    v.z = p2.z - p3.z;
    

    uv = scalprod(u,v);

    nv = norm(v);
    nu = norm(u);

    tmp = uv/(nu*nu);
    h.x = v.x - u.x*tmp;
    h.y = v.y - u.y*tmp;
    h.z = v.z - u.z*tmp;

    nh = norm(h);

    return (nh*nu*0.5);

}