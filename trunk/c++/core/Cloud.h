#ifndef CLOUD_H_
#define CLOUD_H_

#include "Cloud_P.h"

using namespace std;

template <class T = Point>
class Cloud : public Cloud_P
{
public:

  Cloud() : Cloud_P() {}

  Cloud(string filename);

  void draw();

  bool load(istream &in);

  void save(ostream &out);

  void addPoint(float x, float y, float z);

  virtual string className(){
    return "Cloud";
  }

  ~Cloud();

  /** Returns the maximum and minimum coordinates of the cloud. The
      return vector would be in the form: [xmin, xmax, ymin, ymax, zmin, zmax]*/
  vector< double > spread();
};


// FOR NOW, DEFINITIONS HERE, BUT SHOULD CHANGE
template <class T>
 Cloud<T>::Cloud(string filename) : Cloud_P()
{
  if (fileExists(filename))
    loadFromFile(filename);
}

template <class T>
 Cloud<T>::~Cloud()
{
  for(int i = 0; i < points.size(); i++)
    delete(points[i]);
}


template <class T>
 void Cloud<T>::draw(){
  VisibleE::draw();

  if(v_glList == 0){
    // Reduces the number of points to 2000
    float step = points.size() / 2000;
    if(step < 1)
      step = 1;
    else
      printf("Cloud::draw there is a subsampling of the points\n");
    // float step = 1;
    v_glList = glGenLists(1);
    glNewList(v_glList, GL_COMPILE);
    glColor3f(v_r, v_g, v_b);
    // for(float i = 0; i < points.size(); i+=step){
    for(float i = 0; i < points.size(); i++){
      points[(int)i]->draw(v_radius);
    }
    glEndList();
    glCallList(v_glList);
  }
  else{
    glCallList(v_glList);
  }
}

template <class T>
bool Cloud<T>::load(istream &in){
  int start = in.tellg();
  string s;
  in >> s;
  int orig = s.find("<Cloud");
  if(orig == string::npos){
    printf("Cloud::error load called when there is no beginning of Cloud, instead: %s\n",
           s.c_str());
    in.seekg(start);
    return false;
  }
  in >> s;
  T* t = new T();
  orig = s.find(t->className()+">");
  delete t;
  if(orig == string::npos){
    printf("Cloud::error load called when there is no type of the class %s\n", t->className().c_str());
    in.seekg(start);
    return false;
  }

  if(!VisibleE::load(in))
    return false;
  T* p = new T();
  while(p->load(in)){
    points.push_back(p);
    p = new T();
  }
  //delete p;
  in >> s;
  if(s.find("</Cloud>")==string::npos){
    printf("Cloud::error load can not find </Cloud>\n");
    in.seekg(start);
    return false;
  }
  return true;
}

template <class T>
void Cloud<T>::save(ostream &out){
  // printf("Saving the cloud\n");
  T* t = new T();
    out << "<Cloud " << t->className() << ">" << std::endl;
  VisibleE::save(out);
  for(int i = 0; i < points.size(); i++)
    points[i]->save(out);
  out << "</Cloud>" << std::endl;
  delete t;
}

template <class T>
vector<double> Cloud<T>::spread(){
  vector<double> toReturn;
  // string pointType = CloudFactory::inferPointType(this);
  string pointType = "Point3D";
  int nDim = 3;
  // if(pointType.substr("2D") == string::npos){
    // nDim = 2;
  // } else {nDim = 3;}

  double xMax = FLT_MIN;
  double xMin = FLT_MAX;
  double yMax = FLT_MIN;
  double yMin = FLT_MAX;
  double zMax = FLT_MIN;
  double zMin = FLT_MAX;

  for(int i = 0; i < points.size(); i++){
    if(points[i]->coords[0] > xMax) xMax = points[i]->coords[0];
    if(points[i]->coords[0] < xMin) xMin = points[i]->coords[0];
    if(points[i]->coords[1] > yMax) yMax = points[i]->coords[1];
    if(points[i]->coords[1] < yMin) yMin = points[i]->coords[1];
    // if(nDim == 3){
    if(points[i]->coords[2] > zMax) zMax = points[i]->coords[2];
    if(points[i]->coords[2] < zMin) zMin = points[i]->coords[2];
    // }
  }
  // if(nDim == 2){
    // zMax = 0.1;
    // zMin = -0.1;
  // }
  toReturn.push_back(xMin);   toReturn.push_back(xMax);
  toReturn.push_back(yMin);   toReturn.push_back(yMax);
  toReturn.push_back(zMin);   toReturn.push_back(zMax);
  return toReturn;
}

template <class T>
void Cloud<T>::addPoint(float x, float y, float z)
{
  // if( (typeid(T) == typeid(Point2D))   ||
      // (typeid(T) == typeid(Point2Do))  ||
      // (typeid(T) == typeid(Point2Dot)) )
    // points.push_back(new T(x,y));
  // else
    points.push_back(new T(x,y,z));
}

#endif