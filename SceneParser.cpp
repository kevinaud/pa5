/*
 * Parser for XML SceneFile
 */

#include "SceneParser.h"
#include "SceneData.h"
#include "Algebra.h"

#include <string>
#include <iostream>
#include <stdlib.h>
#include <typeinfo>

/*XML Parsing Headers */
#include "tinyxml.h"
#include "tinystr.h"

using namespace std;

SceneParser::SceneParser(const std::string& name) 
{
    file_name = name;

    memset(&m_cameraData, 0, sizeof(SceneCameraData));
    memset(&m_globalData, 0, sizeof(SceneGlobalData));
    m_objects.clear();
    m_lights.clear();
    m_nodes.clear();
}

SceneParser::~SceneParser() 
{
    std::vector<SceneLightData*>::iterator lights;
    for (lights = m_lights.begin(); lights != m_lights.end(); lights++) {
        delete *lights;
    }

    /* Delete all Scene Nodes */
    for (unsigned int node = 0; node < m_nodes.size(); node++) {           
        for (size_t i = 0; i < (m_nodes[node])->transformations.size(); i++) {
            delete (m_nodes[node])->transformations[i];
        }
        for (size_t i = 0; i < (m_nodes[node])->primitives.size(); i++) {
            delete (m_nodes[node])->primitives[i]->material.textureMap;
            delete (m_nodes[node])->primitives[i]->material.bumpMap;
            delete (m_nodes[node])->primitives[i];
        }
        (m_nodes[node])->transformations.clear();
        (m_nodes[node])->primitives.clear();
        (m_nodes[node])->children.clear();
        delete m_nodes[node];
    }

    m_nodes.clear();
    m_lights.clear();
    m_objects.clear(); 
}

bool SceneParser :: getGlobalData(SceneGlobalData& data) 
{
    data = m_globalData;
    return true;
}

bool SceneParser :: getCameraData(SceneCameraData& data) 
{
    data = m_cameraData;
    return true;
}

int SceneParser :: getNumLights()
{
    return m_lights.size();
}

bool SceneParser :: getLightData(int i, SceneLightData& data) 
{
    if (i < 0 || (unsigned int)i >= m_lights.size())
    {
        printf("Invalid light index %d.\n", i);
        return false;
    }
    data = *m_lights[i];
    return true;
}

SceneNode* SceneParser :: getRootNode() 
{
    map<std::string, SceneNode*>::iterator node = m_objects.find("root");
    if (node == m_objects.end())
        return NULL;
    return m_objects["root"];
}

/* this is where it all goes down... */
bool SceneParser::parse() 
{
        //const TiXmlElement* scene_elem = txn->FirstChildElement();

        /* Default Camera */
        m_cameraData.isDir = true;
        m_cameraData.pos = Point(0, 2, 5);
        m_cameraData.up = Vector(0, 1, 0);
        m_cameraData.look = Vector(0, -0.1, 1);
        m_cameraData.lookAt = Point(0, 0, 0);
        m_cameraData.heightAngle = 45;
        m_cameraData.aspectRatio = 1.0f;

        /* Default global data */
        m_globalData.ka = 0.5f;
        m_globalData.kd = 0.2f;
        m_globalData.ks = 0.5f;
        m_globalData.kt = 0.5f;

        // LIGHTS
        SceneLightData* light;
        light = new SceneLightData();
        m_lights.push_back(light);    
        memset(light, 0, sizeof(SceneLightData));
        light->pos = Point(0, 0, 5);
        light->dir = Vector(0, 0, 0);
        light->color.r = 0.5f;
        light->color.g = 0.5f;
        light->color.b = 0.5f;
        light->function = Vector(1, 0, 0);

        light = new SceneLightData();
        m_lights.push_back(light);    
        memset(light, 0, sizeof(SceneLightData));
        light->pos = Point(0, 0, 0);
        light->dir = Vector(0, 0, 0);
        light->color.r = 0.5f;
        light->color.g = 0.5f;
        light->color.b = 0.5f;
        light->function = Vector(1, 0, 0);

        light = new SceneLightData();
        m_lights.push_back(light);    
        memset(light, 0, sizeof(SceneLightData));
        light->pos = Point(0, 0, 5);
        light->dir = Vector(0, 0, 0);
        light->color.r = 0.5f;
        light->color.g = 0.5f;
        light->color.b = 0.5f;
        light->function = Vector(1, 0, 0);

        light = new SceneLightData();
        m_lights.push_back(light);    
        memset(light, 0, sizeof(SceneLightData));
        light->pos = Point(-4, 0, 0);
        light->dir = Vector(0, 0, 0);
        light->color.r = 0.3f;
        light->color.g = 0.3f;
        light->color.b = 0.3f;
        light->function = Vector(1, 0, 0);

        light = new SceneLightData();
        m_lights.push_back(light);    
        memset(light, 0, sizeof(SceneLightData));
        light->pos = Point(0, 4, 0);
        light->dir = Vector(0, 0, 0);
        light->color.r = 0.3f;
        light->color.g = 0.3f;
        light->color.b = 0.3f;
        light->function = Vector(1, 0, 0);

        light = new SceneLightData();
        m_lights.push_back(light);    
        memset(light, 0, sizeof(SceneLightData));
        light->pos = Point(0, -4, 0);
        light->dir = Vector(0, 0, 0);
        light->color.r = 0.3f;
        light->color.g = 0.3f;
        light->color.b = 0.3f;
        light->function = Vector(1, 0, 0);

        /* Create the object and add to the map */
        SceneNode* node = new SceneNode;
        m_nodes.push_back(node);

        m_objects["root"] = node;

        // FIRST CHILD 
        SceneNode* child = new SceneNode;
        m_nodes.push_back(child);

            // translate
            SceneTransformation* t = new SceneTransformation();
            child->transformations.push_back(t);
            t->type = TRANSFORMATION_TRANSLATE;
            t->translate = Vector(0, 0, -5);

            // scale
            t = new SceneTransformation();
            t->type = TRANSFORMATION_SCALE;
            t->scale = Vector(1.5, 1.5, 1.5);
            child->transformations.push_back(t);

            // object
            ScenePrimitive* primitive = new ScenePrimitive();
            SceneMaterial& mat = primitive->material;
            memset(&mat, 0, sizeof(SceneMaterial));
            mat.textureMap = new SceneFileMap;
            mat.bumpMap = new SceneFileMap;

            /* Initialize primitive values */
            primitive->type = SHAPE_CYLINDER;
            mat.textureMap->isUsed = false;
            mat.bumpMap->isUsed = false;
            mat.cDiffuse.r = 0.0;
            mat.cDiffuse.g = 1.0;
            mat.cDiffuse.b = 0.0;

            child->primitives.push_back(primitive);
            node->children.push_back(child);

        // END FIRST CHILD

        // SECOND CHILD

        SceneNode* child1 = new SceneNode;
        m_nodes.push_back(child1);

            // translate
            t = new SceneTransformation();
            child1->transformations.push_back(t);
            t->type = TRANSFORMATION_TRANSLATE;
            t->translate = Vector(-2, 0, -3);

            // scale
            t = new SceneTransformation();
            t->type = TRANSFORMATION_SCALE;
            t->scale = Vector(1.5, 1.5, 1.5);
            child1->transformations.push_back(t);

            // object
            primitive = new ScenePrimitive();
            SceneMaterial& mat1 = primitive->material;
            memset(&mat1, 0, sizeof(SceneMaterial));
            mat1.textureMap = new SceneFileMap;
            mat1.bumpMap = new SceneFileMap;

            /* Initialize primitive values */
            primitive->type = SHAPE_CONE;
            mat1.textureMap->isUsed = false;
            mat1.bumpMap->isUsed = false;
            mat1.cDiffuse.r = 0.0;
            mat1.cDiffuse.g = 0.0;
            mat1.cDiffuse.b = 1.0;

            child1->primitives.push_back(primitive);
            node->children.push_back(child1);

        // END SECOND CHILD

        // SECOND CHILD

        SceneNode* child2 = new SceneNode;
        m_nodes.push_back(child2);

            // translate
            t = new SceneTransformation();
            child2->transformations.push_back(t);
            t->type = TRANSFORMATION_TRANSLATE;
            t->translate = Vector(0, 0, 0);

            // scale
            t = new SceneTransformation();
            t->type = TRANSFORMATION_SCALE;
            t->scale = Vector(1.0, 1.0, 1.0);
            child2->transformations.push_back(t);

            // object
            primitive = new ScenePrimitive();
            SceneMaterial& mat2 = primitive->material;
            memset(&mat2, 0, sizeof(SceneMaterial));
            mat2.textureMap = new SceneFileMap;
            mat2.bumpMap = new SceneFileMap;

            /* Initialize primitive values */
            primitive->type = SHAPE_CUBE;
            mat2.textureMap->isUsed = false;
            mat2.bumpMap->isUsed = false;
            mat2.cDiffuse.r = 1.0;
            mat2.cDiffuse.g = 0.0;
            mat2.cDiffuse.b = 0.0;

            child2->primitives.push_back(primitive);
            node->children.push_back(child2);

        // END SECOND CHILD
        
        // SECOND CHILD

        SceneNode* child3 = new SceneNode;
        m_nodes.push_back(child3);

            // translate
            t = new SceneTransformation();
            child3->transformations.push_back(t);
            t->type = TRANSFORMATION_TRANSLATE;
            t->translate = Vector(0, -2, 0);

            // scale
            t = new SceneTransformation();
            t->type = TRANSFORMATION_SCALE;
            t->scale = Vector(1.0, 1.0, 1.0);
            child3->transformations.push_back(t);

            // object
            primitive = new ScenePrimitive();
            SceneMaterial& mat3 = primitive->material;
            memset(&mat3, 0, sizeof(SceneMaterial));
            mat3.textureMap = new SceneFileMap;
            mat3.bumpMap = new SceneFileMap;

            /* Initialize primitive values */
            primitive->type = SHAPE_SPHERE;
            mat3.textureMap->isUsed = false;
            mat3.bumpMap->isUsed = false;
            mat3.cDiffuse.r = 1.0;
            mat3.cDiffuse.g = 0.0;
            mat3.cDiffuse.b = 0.0;

            child3->primitives.push_back(primitive);
            node->children.push_back(child3);

        // END SECOND CHILD
        

    // END FIRST CHILD
        /* while (scene_elem) */
        /* { */
        /*     const string& elem(scene_elem->Value()); */

        /*     if (!elem.compare("globaldata")) */ 
        /*     { */
        /*         if (!parseGlobalData(scene_elem)) */
        /*             return false; */
        /*     } */
        /*     else if (!elem.compare("lightdata")) */
        /*     { */
        /*         if (!parseLightData(scene_elem)) */
        /*             return false; */
        /*     } */
        /*     else if (!elem.compare("cameradata")) */
        /*     { */
        /*         if (!parseCameraData(scene_elem)) */
        /*             return false; */
        /*     } */
        /*     else if (!elem.compare("object")) */
        /*     { */
        /*         if (!parseObjectData(scene_elem)) */
        /*             return false; */
        /*     } */
        /*     else */
        /*     { */
        /*         printf("Unsupported element type %s\n", elem.c_str()); */
        /*         return false; */
        /*     } */

        /*     /1* advance to the next scene element *1/ */
        /*     scene_elem = scene_elem->NextSiblingElement(); */
        /* } */

        printf("Finished parsing file.\n");
        return true;

}

/** Helper function to parse a triple 
*/
bool parseTriple(const TiXmlElement* triple, float& x, float& y, float& z)
{
    const TiXmlAttribute* attr = triple->FirstAttribute();
    if (!attr)
        return false;

    double temp;

    triple->Attribute("x", &temp);
    x = temp;

    triple->Attribute("y", &temp);
    y = temp;

    triple->Attribute("z", &temp);
    z = temp;

    return true;
}
bool parseMatrix(const TiXmlElement* e, double matrix[16]) {
    const TiXmlElement* r = e->FirstChildElement();

    for (int row = 0; row < 4; row++) {
        if (!r)
            goto end;
        const TiXmlAttribute* a = r->FirstAttribute();

        for (int col = 0; col < 4; col++) {
            if (!a)
                goto end;
            matrix[row*4 + col] = atof(a->Value());
            a = a->Next();
        }
        r = r->NextSiblingElement();
    }
    return true;
end:
    printf("Could not parse matrix.\n");
    return false;
}

bool parseColor(const TiXmlElement* color, SceneColor& c)
{
    memset(&c, 0, sizeof(SceneColor));

    const TiXmlAttribute* attr = color->FirstAttribute();
    if (!attr)
        return false;

    double temp;
    color->Attribute("r", &temp);
    c.r = temp;

    color->Attribute("g", &temp);
    c.g = temp;

    color->Attribute("b", &temp);
    c.r = temp;

    c.a = 1.0f;

    return true;
}

bool parseFloat(const TiXmlElement* value, float& f)
{
    const TiXmlAttribute* attr = value->FirstAttribute();
    if (!attr)
        return false;
    double temp;
    value->Attribute("v", &temp);
    f = temp;
    /* int ival; */
    /* double dval; */
    /* cout << "test1" << endl; */
    /* if (attr->QueryIntValue(&ival)==TIXML_SUCCESS) { */
    /*     printf( " int=%d", ival); */
    /* } else { */
    /*     cout << "int failure" << endl; */ 
    /* } */
    /* cout << "test2" << endl; */
    /* if (attr->QueryDoubleValue(&dval)==TIXML_SUCCESS) { */
    /*     printf( " d=%1.1f", dval); */
    /* } else { */
    /*     cout << "int failure" << endl; */ 
    /* } */
    /* cout << temp << endl; */
    /* cout << string(attr->Name()) << endl; */
    /* cout << typeid(attr->Name()).name() << endl; */
    /* f = atof(attr->Value()); */
    return true;
}

bool parseMap(const TiXmlElement* e, SceneFileMap* map)
{
    const TiXmlAttribute* attr = e->FirstAttribute();
    if (!attr)
        return false;

    map->filename = string(attr->Value());

    attr = attr->Next();
    if (attr)
    {
        map->repeatU = atof(attr->Value());
        attr = attr->Next();
        if (!attr)
            return false;
        map->repeatV = atof(attr->Value());
    }
    else
        map->repeatU = map->repeatV = 1.0f;

    map->isUsed = true;

    return true;
}

/**
 *
 * The initial XML element passed in as an argument SHOULD be
 * the element for the data being parsed. In this case, it should
 * be the GlobalData element.
 *
 */
bool SceneParser::parseGlobalData(const TiXmlElement* scenefile) 
{

    const TiXmlElement* globaldata = scenefile->FirstChildElement();

    if (!globaldata) 
    {
        printf("Could not parse global data.\n");
        return false;
    }

    while (globaldata) 
    {
        const string& elem(globaldata->Value());
        if (!elem.compare( "diffusecoeff"))
        {
            if (!parseFloat(globaldata, m_globalData.kd))
            {
                printf("Invalid diffuse coefficient.\n");
                return false;
            }
        }
        else if (!elem.compare("ambientcoeff"))
        {
            if (!parseFloat(globaldata, m_globalData.ka))
            {
                printf("Invalid ambient coefficient.\n");
                return false;
            }
        }
        else if (!elem.compare("transparentcoeff"))
        {
            if (!parseFloat(globaldata, m_globalData.kt))
            {
                printf("Invalid transparent coefficient.\n");
                return false;
            }
        }
        else if (!elem.compare("specularcoeff"))
        {
            if (!parseFloat(globaldata, m_globalData.ks))
            {
                printf("Invalid specular coefficient.\n");
                return false;
            }
        }
        else 
        {
            printf("Invalid global data type %s\n", elem.c_str());
            return false;
        }
        globaldata = globaldata->NextSiblingElement();
    }
    return true;
}



/**
 *
 * The initial XML element passed in as an argument SHOULD be
 * the element for the data being parsed. In this case, it should
 * be the LightData element.
 *
 */

bool SceneParser::parseLightData(const TiXmlElement* scenefile) 
{
    const TiXmlElement* lightelem = scenefile->FirstChildElement();

    if (!lightelem) 
    {
        printf("Invalid light data - no data provided.\n");
        return false;
    }

    SceneLightData* light = new SceneLightData();
    m_lights.push_back(light);    
    memset(light, 0, sizeof(SceneLightData));
    light->pos = Point(3, 3, 3);
    light->dir = Vector(0, 0, 0);
    light->color.r = 0.5f;
    light->color.g = 0.5f;
    light->color.b = 0.5f;
    light->function = Vector(1, 0, 0);

    while(lightelem) 
    {
        const string& elem(lightelem->Value());

        if (!elem.compare("id")) 
        {
            const TiXmlAttribute* attr = lightelem->FirstAttribute();
            if (!attr)
            {
                printf("Invalid light id.\n");
                return false;
            }
            light->id  = strtol(attr->Value(), NULL, 10);        
        }
        else if (!elem.compare("type"))
        {
            const TiXmlAttribute* attr = lightelem->FirstAttribute();
            if (!attr)
            {
                printf("Invalid light type.\n");
                return false;
            }
            string val(attr->Value());
            if (!val.compare("directional"))
                light->type = LIGHT_DIRECTIONAL;
            else if (!val.compare("point"))
                light->type = LIGHT_POINT;
            else if (!val.compare("spot"))
                light->type = LIGHT_SPOT;
            else if (!val.compare("area"))
                light->type = LIGHT_AREA;
            else {
                printf("Unknown light type %s.\n", val.c_str());
                return false;
            }

        }
        else if (!elem.compare("color"))
        {
            if (!parseColor(lightelem, light->color))
            {
                printf("Invalid light color.\n");
                return false;
            }
        }
        else if (!elem.compare("function"))
        {
            float x, y, z;
            if (parseTriple(lightelem, x, y, z))
                light->function = Vector(x, y, z);
            else
            {
                printf("Invalid light function.\n");
                return false;
            }
        }
        else if (!elem.compare("position"))
        {
            if (light->type == LIGHT_DIRECTIONAL)
            {
                printf("Position is not applicable to directional lights.\n");
                return false;
            }
            float x, y, z;
            if (parseTriple(lightelem, x, y, z))
                light->pos = Point(x, y, z);
            else
            {
                printf("Invalid light position.\n");
                return false;
            }
        }    
        else if (!elem.compare("direction"))
        {
            if (light->type == LIGHT_POINT)
            {
                printf("Direction is not applicable to point lights.\n");
                return false;
            }
            float x, y, z;
            if (parseTriple(lightelem, x, y, z))
                light->dir = Vector(x, y, z);
            else
            {
                printf("Invalid light direction.\n");
                return false;
            }
        }
        else if (!elem.compare("radius"))
        {
            if (light->type != LIGHT_SPOT)
            {
                printf("Radius is only applicable to spot lights.\n");
                return false;
            }
            if (!parseFloat(lightelem, light->radius))
            {
                printf("Invalid radius.\n");
                return false;
            }
        }
        else if (!elem.compare("penumbra"))
        {
            if (light->type != LIGHT_SPOT)
            {
                printf("Penumbra is only applicable to spot lights.\n");
                return false;
            }
            if (!parseFloat(lightelem, light->penumbra))
            {
                printf("Invalid penumbra.\n");
                return false;
            }
        }
        else if (!elem.compare("angle"))
        {
            if (light->type != LIGHT_SPOT)
            {
                printf("Angle is only applicable to spot lights.\n");
                return false;
            }
            if (!parseFloat(lightelem, light->angle))
            {
                printf("Invalid angle.\n");
                return false;
            }
        }
        else if (!elem.compare("width"))
        {
            if (light->type != LIGHT_AREA)
            {
                printf("Width is only applicable to area lights.\n");
                return false;
            }
            if (!parseFloat(lightelem, light->width))
            {
                printf("Invalid width.\n");
                return false;
            }
        }
        else if (!elem.compare("height"))
        {
            if (light->type != LIGHT_AREA)
            {
                printf("Height is only applicable to area lights.\n");
                return false;
            }
            if (!parseFloat(lightelem, light->height))
            {
                printf("Invalid height.\n");
                return false;
            }
        }
        else {
            printf("Invalid element type %s.\n", elem.c_str());
        }

        lightelem = lightelem->NextSiblingElement();
    }

    return true;
}


/**
 *
 * The initial XML element passed in as an argument SHOULD be
 * the element for the data being parsed. In this case, it should
 * be the CameraData element.
 *
 */
bool SceneParser::parseCameraData(const TiXmlElement* scenefile)
{
    const TiXmlElement* cam_element = scenefile->FirstChildElement();
    if (!cam_element)
    {
        printf("Invalid camera data.\n");
        return false;
    }

    m_cameraData.isDir = true;

    bool focusFound = false;
    bool lookFound = false;

    while(cam_element)
    {
        const string& elem(cam_element->Value());
        if (!elem.compare("pos"))
        {
            float x, y, z;
            if (!parseTriple(cam_element, x, y, z))
            {
                printf("Invalid camera position.\n");
                return false;
            }
            m_cameraData.pos = Point(x, y, z);
        }
        else if (!elem.compare("look"))
        {
            float x, y, z;
            if (!parseTriple(cam_element, x, y, z))
            {
                printf("Invalid camera look.\n");
                return false;
            }
            lookFound = true;
            m_cameraData.isDir = true;
            m_cameraData.look = Vector(x, y, z);
        }
        else if (!elem.compare("focus"))
        {
            float x, y, z;
            if (!parseTriple(cam_element, x, y, z))
            {
                printf("Invalid camera focus.\n");
                return false;
            }
            m_cameraData.lookAt = Point (x, y, z);
            m_cameraData.isDir = false;
            focusFound = true;
        }
        else if (!elem.compare("up"))
        {
            float x, y, z;
            if (!parseTriple(cam_element, x, y, z))
            {
                printf("Invalid camera up.\n");
                return false;
            }
            m_cameraData.up = Vector(x, y, z);
        }
        else if (!elem.compare("heightangle"))
        {
            if (!parseFloat(cam_element, m_cameraData.heightAngle))
            {
                printf("Invalid camera field of view.\n");
                return false;
            }    
        }
        else if (!elem.compare("aspectratio"))
        {
            if (!parseFloat(cam_element, m_cameraData.aspectRatio))
            {
                printf("Invalid camera aspect ratio.\n");
                return false;
            }
        }        
        else if (!elem.compare("aperture"))
        {
            if (!parseFloat(cam_element, m_cameraData.aperture))
            {
                printf("Invalid camera aperture.\n");
                return false;
            }
        }        
        else if (!elem.compare("focallength"))
        {
            if (!parseFloat(cam_element, m_cameraData.focalLength))
            {
                printf("Invalid camera focal length.\n");
                return false;
            }
        }        
        else 
        {
            printf("Invalid camera element %s.\n", elem.c_str());
            return false;
        }
        cam_element = cam_element->NextSiblingElement();
    }

    if (focusFound && lookFound)
    {
        printf("Camera can not have both look and focus.\n");
        return false;
    }
    //if (focusFound)
    //{
    //   m_cameraData.look[0] = m_cameraData.look[0] - m_cameraData.pos[0];
    //   m_cameraData.look[1] = m_cameraData.look[1] - m_cameraData.pos[1];
    //   m_cameraData.look[2] = m_cameraData.look[2] - m_cameraData.pos[2];
    //   m_cameraData.look[3] = 0.0f;
    //}

    return true;
}

/*
 * This method expects that the element passed in was an object
 * Further, it is almost certainly a tree object, since the only caller
 * of this method will be the element loop in the parse method.
 *
 */
bool SceneParser::parseObjectData(const TiXmlElement* scenefile){

    const TiXmlAttribute* attr = scenefile->FirstAttribute();

    string val(attr->Value());
    if (attr == NULL || val.compare("tree"))
    {
        printf("Invalid object data.\n");
        return false;\
    }

    const TiXmlAttribute* name = scenefile->FirstAttribute()->Next();
    const string& nameStr(name->Value());

    /* now we iterate over its child elements, which MUST be trans blocks.
     * master objects contain no object blocks within themselves.
     */
    const TiXmlElement* obj_element = scenefile->FirstChildElement();
    if (!obj_element)
    {
        printf("Object has no data.\n");
        return false;
    }

    /* Check that this object does not exist */
    if (m_objects[nameStr] != NULL)
    {
        printf("Two objects with the same name: %s.\n", nameStr.c_str());
        return false;
    }


    /* /1* now iterate over all of the transblocks in this master object *1/ */
    /* while (obj_element) */
    /* { */
    /*     SceneNode* child = new SceneNode; */
    /*     m_nodes.push_back(child); */

    /*     if (!parseTransBlock(obj_element, child)) */
    /*     { */
    /*         printf("Could not parse transblock.\n"); */
    /*         return false; */
    /*     } */
    /*     node->children.push_back(child); */
    /*     /1* advance the element pointer *1/ */
    /*     obj_element = obj_element->NextSiblingElement(); */
    /* } */

    return true;
}

/**
 *
 * This method expects a transblock as its element, so we need
 * to iterate over all of its child elements, which could be 
 * translate, rotate, etc etc elements.  The information will be stripped
 * from the element, and then we'll tackle the next one. If it's an object
 * in here, it's either a master reference, a subtree, or a primitive.
 * if it's a master reference, we handle it here. if it's one of the other
 * two, then we call other methods.
 *
 */

bool SceneParser::parseTransBlock(const TiXmlElement* transblock, SceneNode* node)
{
    /* could have any number of translates, rotates, scale, matrix */
    /* and then one final object block */
    const TiXmlElement* trans_elem = transblock->FirstChildElement();
    if (!trans_elem)
    {
        printf("No transformation data.\n");
        return false;
    }

    while(trans_elem)
    {
        const string& elem(trans_elem->Value());

        if (!elem.compare("translate"))
        {
            SceneTransformation* t = new SceneTransformation();
            node->transformations.push_back(t);
            t->type = TRANSFORMATION_TRANSLATE;
            float x, y, z;

            if (!parseTriple(trans_elem, x, y, z))
            {
                printf("Could not parse translate type.\n");
                return false;
            }

            t->translate = Vector(x, y, z);
        }
        else if (!elem.compare("rotate"))
        {
            SceneTransformation* t = new SceneTransformation();
            t->type = TRANSFORMATION_ROTATE;
            float x, y, z, angle = 0;

            const TiXmlAttribute* attr = trans_elem->FirstAttribute();
            if (!attr)
                return false;

            x = atof(attr->Value());
            attr = attr->Next();
            if (!attr)
                return false;

            y = atof(attr->Value());
            attr = attr->Next();
            if (!attr)
                return false;

            z = atof(attr->Value());
            attr = attr->Next();
            if (attr)
            {
                angle = atof(attr->Value());
            }

            t->rotate = Vector(x, y, z);
            /* Convert to radians */
            t->angle = angle * PI / 180.f;

            node->transformations.push_back(t);   
        }
        else if (!elem.compare("scale"))
        {
            SceneTransformation* t = new SceneTransformation();
            t->type = TRANSFORMATION_SCALE;
            float x, y, z;

            if (!parseTriple(trans_elem, x, y, z))
            {
                printf("Could not parse scale type.\n");
                return false;
            }

            t->scale = Vector(x, y, z);
            node->transformations.push_back(t);
        }
        else if (!elem.compare("matrix"))
        {
            SceneTransformation* t = new SceneTransformation();
            t->type = TRANSFORMATION_MATRIX;

            double matrix[16];
            if (!parseMatrix(trans_elem, matrix)) {
                printf("Could not parse matrix.\n");
                return false;
            }

            t->matrix = Matrix(matrix);
            node->transformations.push_back(t);
        }

        else if (!elem.compare("object"))
        {
            const TiXmlAttribute* attr = trans_elem->FirstAttribute();
            string val(attr->Value());
            if (!val.compare("master"))
            {
                attr = attr->Next();
                if (!attr)
                {
                    printf("Invalid master object reference.\n");
                    return false;
                }
                if (!m_objects[val])
                {
                    printf("Invalid master object reference %s\n", 
                            val.c_str());
                    return false;
                }
                node->children.push_back(m_objects[val]);
            }
            else if (!val.compare("tree"))
            {
                const TiXmlElement* subblock = trans_elem->FirstChildElement();
                while (subblock)
                {
                    SceneNode* n = new SceneNode;
                    m_nodes.push_back(n);
                    if (!parseTransBlock(subblock, n))
                        return false;
                    node->children.push_back(n);
                    subblock = subblock->NextSiblingElement();
                }
            }
            else if (!val.compare("primitive"))
            {
                if (!parsePrimitive(trans_elem, node))
                {
                    printf("Could not parse primitive.\n");
                    return false;
                }
            }
            else {
                printf("Unknown object type!.\n");
                return false;
            }
        }
        else 
        {
            printf("Invalid transblock element type %s\n", elem.c_str());
            return false;
        }
        trans_elem = trans_elem->NextSiblingElement();
    }

    return true;
}

/**
 * Parse primitive data 
 */
bool SceneParser::parsePrimitive(const TiXmlElement* prim, SceneNode* node)
{
    const TiXmlAttribute* attr = prim->FirstAttribute();
    attr = attr->Next();
    if (!prim)
    {
        printf("Could not parse primitive type.\n");
        return false;
    }

    ScenePrimitive* primitive = new ScenePrimitive();
    SceneMaterial& mat = primitive->material;
    memset(&mat, 0, sizeof(SceneMaterial));
    mat.textureMap = new SceneFileMap;
    mat.bumpMap = new SceneFileMap;

    /* Initialize primitive values */
    primitive->type = SHAPE_CUBE;
    mat.textureMap->isUsed = false;
    mat.bumpMap->isUsed = false;
    mat.cDiffuse.r = mat.cDiffuse.g = mat.cDiffuse.b = 1.0;

    node->primitives.push_back(primitive);

    const string& primType(attr->Value());

    if (!primType.compare("sphere"))
        primitive->type = SHAPE_SPHERE;
    else if (!primType.compare("cube"))
        primitive->type = SHAPE_CUBE;
    else if (!primType.compare("cylinder"))
        primitive->type = SHAPE_CYLINDER;
    else if (!primType.compare("cone"))
        primitive->type = SHAPE_CONE;
    else if (!primType.compare("special1"))
        primitive->type = SHAPE_SPECIAL1;
    else if (!primType.compare("special2"))
        primitive->type = SHAPE_SPECIAL2;
    else if (!primType.compare("special3"))
        primitive->type = SHAPE_SPECIAL3;
    else if (!primType.compare("mesh"))
    {
        primitive->type = SHAPE_MESH;
        attr = attr->Next();
        if (!attr)
        {
            printf("Mesh type must specify mesh file.\n");
            return false;
        }
        primitive->meshfile = string(attr->Value());
    }
    const TiXmlElement* primdata = prim->FirstChildElement();

    while(primdata)
    {
        const string& elem = string(primdata->Value());

        if (!elem.compare("diffuse"))
        {
            if (!parseColor(primdata, mat.cDiffuse))
            {
                printf("Could not parse diffuse color.\n");
                return false;
            }
        }
        else if (!elem.compare("ambient"))
        {
            if (!parseColor(primdata, mat.cAmbient))
            {
                printf("Could not parse ambient color.\n");
                return false;
            }
        }
        else if (!elem.compare("reflective"))
        {
            if (!parseColor(primdata, mat.cReflective))
            {
                printf("Could not parse reflective color.\n");
                return false;
            }
        }
        else if (!elem.compare("specular"))
        {
            if (!parseColor(primdata, mat.cSpecular))
            {
                printf("Could not parse specular color.\n");
                return false;
            }
        }
        else if (!elem.compare("emissive"))
        {
            if (!parseColor(primdata, mat.cEmissive))
            {
                printf("Could not parse emissive color.\n");
                return false;
            }
        }
        else if (!elem.compare("transparent"))
        {
            if (!parseColor(primdata, mat.cTransparent))
            {
                printf("Could not parse transparent color.\n");
                return false;
            }
        }
        else if (!elem.compare("shininess"))
        {
            if (!parseFloat(primdata, mat.shininess))
            {
                printf("Could not parse shininess.\n");
                return false;
            }
        }
        else if (!elem.compare("ior"))
        {
            if (!parseFloat(primdata, mat.ior))
            {
                printf("Could not parse ior.\n");
                return false;
            }
        }
        else if (!elem.compare("texture"))
        {
            if (!parseMap(primdata, mat.textureMap))
            {
                printf("Could not parse texture map.\n");
                return false;
            }
        }
        else if (!elem.compare("bumpmap"))
        {
            if (!parseMap(primdata, mat.bumpMap))
            {
                printf("Could not parse bump map.\n");
                return false;
            }
        }
        else if (!elem.compare("blend"))
        {
            if (!parseFloat(primdata, mat.blend))
            {
                printf("Could not parse blend.\n");
                return false;
            }
        }
        else 
        {
            printf("Invalid primitive data %s\n", elem.c_str());
            return false;
        }
        primdata = primdata->NextSiblingElement();
    }

    return true;
}

