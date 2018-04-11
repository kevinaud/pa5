#ifndef TEXTURE_MAP_H
#define TEXTURE_MAP_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Algebra.h"

using namespace std;

class TextureMap {
    public: 
        TextureMap() {
        
        }

        TextureMap(string filename) {
            file = ifstream(filename); 
            parseHeader();
            allocateMap();
            parseRGBValues();
            file.close();
        } 

        ~TextureMap() {
            for (int i = 0; i < width; i++) {
                delete[] textureMap[i]; 
            } 
            delete[] textureMap;
        }

        /**
         * u and v should be >= 0 and <= 1
         */
        Point valueAt(double u, double v) {
            int x = (int)(u * width);
            int y = (int)((1.0 - v) * height);
            if (x < 0) {
                x = 0; 
            }
            if (y < 0) {
                y = 0; 
            }
            if (x > (width - 1)) {
                x = width - 1; 
            }
            if (v > (height - 1)) {
                v = height - 1; 
            }
            return textureMap[x][y]; 
        }

    private: 
        ifstream file;
        Point** textureMap; 

        int height, width;

        void parseHeader() {
            string line;
            getline(file, line); 
            getline(file, line); 

            file >> width >> height;
        }

        void allocateMap() {
            textureMap = new Point*[width];
            for (int i = 0; i < width; i++) {
                textureMap[i] = new Point[height]; 
            } 
        }
        
        void parseRGBValues() {
            double val;
            for (int j = 0; j < height; j++) {
                for (int i = 0; i < width; i++) {
                    for (int k = 2; k >= 0; k--) {
                        file >> val;
                        textureMap[i][j][k] = val / 256.0; 
                    }
                } 
            } 
        }
};

#endif
