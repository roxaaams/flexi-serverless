#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

int main() {

    ifstream image;
    ofstream newImage;

    image.open("old_images/image.ppm");
    newImage.open("newImage.ppm");

    // copy over header information
    string type = "", width = "", height = "", RGB = "";
    image >> type >> width >> height >> RGB;



    string red = "", green = "", blue = "";
    int r = 0, g = 0, b = 0;
    while (!image.eof()) {
        image >> red >> green >> blue;

        stringstream redStream(red);
        stringstream greenStream(green);
        stringstream blueStream(green);

        redStream >> r;
        greenStream >> g;
        blueStream >> b;

        if (b + 50 > 255) {
            b = 255;
        } else {
            b += 50;
        }

        newImage << r << " " << g << " " << b << endl;
    }
}
