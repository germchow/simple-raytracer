#include <iostream>
#include <vector>
#include <math.h>
#include <fstream>

const int IMAGE_WIDTH = 64, IMAGE_HEIGHT = 64;
const int INF = 2147483647; // Infinity is set to max possible int value (32bits)
const float PI = 3.1415926535897931;

class Vec3 {
    public:
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {};
    Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {};

    float length() {
        return sqrtf(x*x + y*y + z*z);
    }
    Vec3& normalize() {
        if (length() > 0) {
            float inverseL = 1 / length();
            x *= inverseL;
            y *= inverseL;
            z *= inverseL;  
        }   
        return *this;
    } 
    float dot(Vec3 v) {
        return x * v.x + y * v.y + z * v.z;
    }
    Vec3 vecAddition(Vec3 v) {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }
    Vec3 vecSubtraction(Vec3 v) {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }
    Vec3 scalarMultiply(float f) {
        return Vec3(f * x, f * y, f * z);
    }
};

class Sphere {
    public:
    Vec3 center;
    float radius, radius2;
    Vec3 color, emissionColor; // Colours are represented as 3Vectors, [R, G, B]
    
    Sphere(Vec3 _cen, float _rad, Vec3 _color, Vec3 _em) : 
    center(_cen), radius(_rad), color(_color), emissionColor(_em) {
        radius2 = _rad*_rad;
    }
};

bool findIntersection(Vec3 origin, Vec3 direction, Sphere &s, float &t0, float &t1) {
    // Formulas from https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
    Vec3 l = s.center.vecSubtraction(origin);
    float tca = direction.dot(l);
    if (tca < 0) return false;
    float d2 = sqrtf(l.dot(l) - tca * tca);
    if (d2 > s.radius2) return false;
    float thc = sqrtf(s.radius2 - d2);
    t0 = tca - thc;
    t1 = tca + thc;
    return true;
}

Vec3 trace(Vec3 origin, Vec3 direction, std::vector<Sphere> objects) {
    float tNearest = INF;
    Sphere *hit = NULL;
    for (int i = 0; i < objects.size(); i++) {
        float t0 = INF, t1 = INF;
        if (findIntersection(origin, direction, objects[i], t0, t1)) {
            if (t0 > 0 && t0 < tNearest) { 
                tNearest = t0; 
                hit = &objects[i]; 
            } 
        }
    }
    if (hit == NULL) return Vec3(1,1,1);

    Vec3 colorReturned(0,0,0);
    Vec3 pint = origin.vecAddition(direction.scalarMultiply(tNearest));
    Vec3 pnorm = pint.vecSubtraction(hit->center);
    pnorm.normalize();

    // Diffuse Objects
    for (int i = 0; i < objects.size(); i++) {
       if (objects[i].emissionColor.x > 0 || objects[i].emissionColor.y > 0 || objects[i].emissionColor.z > 0) {
            Vec3 dir = objects[i].center.vecSubtraction(pint);
            dir.normalize();
            Vec3 cont = hit->color.scalarMultiply(std::max(float(0), pnorm.dot(dir)) * objects[i].emissionColor.dot(Vec3(1,1,1)));
            for (int j = 0; j < objects.size(); j++) {
                if (i != j) {
                    float t0, t1;
                    if (findIntersection(pint, dir, objects[j], t0, t1)) {
                        cont = Vec3(0,0,0);
                        break;
                    }
                }
            }
            colorReturned = colorReturned.vecAddition(cont);
        } 
    }
    return colorReturned;
}

void render(std::vector<Sphere> objects) {
    Vec3 *img = new Vec3[IMAGE_HEIGHT * IMAGE_WIDTH];
    Vec3 origin;
    int imgIndex = 0;
    for (int i = 0; i < IMAGE_HEIGHT; i++) {
        for (int j = 0; j < IMAGE_WIDTH; j++) {
            float x = (j + 0.5 - IMAGE_HEIGHT / 2) / IMAGE_WIDTH;
            float y = (i + 0.5 - IMAGE_WIDTH / 2) / IMAGE_WIDTH;
            Vec3 primaryRay(x, y, 1); // Changing z value affects the FOV
            img[imgIndex] = trace(origin, primaryRay.normalize(), objects);
            imgIndex++;
        }
    }

    // Outputting the resulting image as a ppm file (Borrowed code)
    std::ofstream ofs("./output.ppm", std::ios::out | std::ios::binary);
    ofs << "P6\n" << IMAGE_WIDTH << " " << IMAGE_HEIGHT << "\n255\n";
    for (unsigned i = 0; i < IMAGE_WIDTH * IMAGE_HEIGHT; ++i) { 
        ofs << (unsigned char)(std::min(float(1), img[i].x) * 255) << 
               (unsigned char)(std::min(float(1), img[i].y) * 255) << 
               (unsigned char)(std::min(float(1), img[i].z) * 255); 
    } 
    ofs.close(); 
    delete [] img;
}

int main() { 
    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3(0,0,5), 1.2, Vec3(.1,.3,.3), Vec3(0,0,0)));
    spheres.push_back(Sphere(Vec3(-6,-6,-2), 1, Vec3(0,0,0), Vec3(1,1,1)));

    render(spheres);
}