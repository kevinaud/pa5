# The rest should be okay!
GLUT_LIB=/usr/lib
GLUT_INCLUDE=/usr/include
GLUI_LIB=lib
GLUI_INCLUDE=.

FLAGS = -lglut -lGL -lGLU -lglui 

SHAPE_HEADERS = Cone.h Cube.h Cylinder.h Sphere.h Torus.h 
XML_HEADERS = tinyxml.o tinyxmlparser.o tinyxmlerror.o tinystr.o

Assignment5: Assignment5.o Shapes.o Camera.o SceneParser.o TextureMap.o $(XML_HEADERS)
	g++ -DTIXML_USE_STL -fPIC -o Assignment5 Camera.o Assignment5.o SceneParser.o $(XML_HEADERS) $(FLAGS) 

Assignment5.o: Assignment5.cpp $(SHAPE_HEADERS) Camera.h
	g++ -fPIC -c Assignment5.cpp

Shapes.o: Shape.o $(SHAPE_HEADERS)
	g++ -fPIC -g -c $(SHAPE_HEADERS)

Shape.o: Algebra.o Camera.o Shape.h
	g++ -fPIC -g -c Shape.h Camera.cpp

Algebra.o: Algebra.h
	g++ -fPIC -g -c Algebra.h

Camera.o: Camera.h Camera.cpp
	g++ -fPIC -g -c Camera.cpp

SceneParser.o: SceneParser.h SceneParser.cpp xml.o TextureMap.o
	g++ -fPIC -g -c SceneParser.h SceneParser.cpp

TextureMap.o: TextureMap.h Algebra.o
	g++ -fPIC -g -c TextureMap.h -o TextureMap.o

xml.o: $(XML_HEADERS)

tinyxml.o: tinyxml.h tinystr.h
	g++ -c -DTIXML_USE_STL -Wall -Wno-unknown-pragmas -Wno-format -O3 tinyxml.cpp -o tinyxml.o

tinyxmlparser.o: tinyxml.h tinystr.h
	g++ -c -DTIXML_USE_STL -Wall -Wno-unknown-pragmas -Wno-format -O3 tinyxmlparser.cpp -o tinyxmlparser.o

tinyxmlerror.o: tinyxml.h tinystr.h
	g++ -c -DTIXML_USE_STL -Wall -Wno-unknown-pragmas -Wno-format -O3 tinyxmlerror.cpp -o tinyxmlerror.o

tinystr.o: tinystr.h
	g++ -c -DTIXML_USE_STL -Wall -Wno-unknown-pragmas -Wno-format -O3 tinystr.cpp -o tinystr.o
