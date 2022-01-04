///////////////////////////////////////////////////
//
//  Hamish Carr
//  September, 2020
//
//  ------------------------
//  AttributedObject.cpp
//  ------------------------
//  
//  Base code for rendering assignments.
//
//  Minimalist (non-optimised) code for reading and 
//  rendering an object file
//  
//  We will make some hard assumptions about input file
//  quality. We will not check for manifoldness or 
//  normal direction, &c.  And if it doesn't work on 
//  all object files, that's fine.
//
//	Variant on TexturedObject that stores explicit RGB
//	values for each vertex
//  
///////////////////////////////////////////////////

// include the header file
#include "AttributedObject.h"

// include the C++ standard libraries we want
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>

// include the Cartesian 3- vector class
#include "Cartesian3.h"

#define MAXIMUM_LINE_LENGTH 1024
#define REMAP_TO_UNIT_INTERVAL(x) (0.5 + (0.5*(x)))
#define REMAP_FROM_UNIT_INTERVAL(x) (-1.0 + (2.0*(x)))

// constructor will initialise to safe values
AttributedObject::AttributedObject()
    : centreOfGravity(0.0,0.0,0.0)
    { // AttributedObject()
    // force arrays to size 0
    vertices.resize(0);
    colours.resize(0);
    normals.resize(0);
    textureCoords.resize(0);
    faceVertices.resize(0);
    faceColours.resize(0);
    faceNormals.resize(0);
    faceTexCoords.resize(0);
    } // AttributedObject()

// read routine returns true on success, failure otherwise
bool AttributedObject::ReadObjectStream(std::istream &geometryStream)
    { // ReadObjectStream()
    
    // create a read buffer
    char readBuffer[MAXIMUM_LINE_LENGTH];
    
    // the rest of this is a loop reading lines & adding them in appropriate places
    while (true)
        { // not eof
        // character to read
        char firstChar = geometryStream.get();
        
//         std::cout << "Read: " << firstChar << std::endl;
        
        // check for eof() in case we've run out
        if (geometryStream.eof())
            break;

        // otherwise, switch on the character we read
        switch (firstChar)
            { // switch on first character
            case '#':       // comment line
                // read and discard the line
                geometryStream.getline(readBuffer, MAXIMUM_LINE_LENGTH);
                break;
                
            case 'v':       // vertex data of some type
                { // some sort of vertex data
                // retrieve another character
                char secondChar = geometryStream.get();
                
                // bail if we ran out of file
                if (geometryStream.eof())
                    break;

                // now use the second character to choose branch
                switch (secondChar)
                    { // switch on second character
                    case ' ':       // space - indicates a vertex
                        { // vertex read
                        Cartesian3 vertex;
                        geometryStream >> vertex;
                        vertices.push_back(vertex);
//                         std::cout << "Vertex " << vertex << std::endl;
                        break;
                        } // vertex read
                    case 'c':       // c indicates colour
                        { // normal read
                        Cartesian3 colour;
                        geometryStream >> colour;
                        colours.push_back(colour);
//                         std::cout << "Colour " << colour << std::endl;
                        break;
                        } // normal read
                    case 'n':       // n indicates normal vector
                        { // normal read
                        Cartesian3 normal;
                        geometryStream >> normal;
                        normals.push_back(normal);
//                         std::cout << "Normal " << normal << std::endl;
                        break;
                        } // normal read
                    case 't':       // t indicates texture coords
                        { // tex coord
                        Cartesian3 texCoord;
                        geometryStream >> texCoord;
                        textureCoords.push_back(texCoord);
//                         std::cout << "Tex Coords " << texCoord << std::endl;
                        break;                  
                        } // tex coord
                    default:
                        break;
                    } // switch on second character 
                break;
                } // some sort of vertex data
                
            case 'f':       // face data
                { // face
				// make a hard assumption that we have a single triangle per line
                unsigned int vertexID;
				unsigned int colourID;
                unsigned int normalID;
				unsigned int texCoordID;				
                
                // read in three vertices
				for (unsigned int vertex = 0; vertex < 3; vertex++)
					{ // per vertex
					// read a vertex ID
					geometryStream >> vertexID;
					// read and discard the slash
					geometryStream.get();
					// read a colour ID
					geometryStream >> colourID;
					// read and discard the slash
					geometryStream.get();
					// read a vertex ID
					geometryStream >> texCoordID;
					// read and discard the slash
					geometryStream.get();
					// read a vertex ID
					geometryStream >> normalID;

// 					std::cout << "Face " << vertexID << "/" << colourID << "/" << texCoordID << "/" << normalID << std::endl;

					// subtract one and store them (OBJ uses 1-based numbering)
					faceVertices.push_back(vertexID-1);
					faceColours.push_back(colourID-1);
					faceNormals.push_back(normalID-1);
					faceTexCoords.push_back(texCoordID-1);
					} // per vertex
				break;
                } // face
                
            // default processing: do nothing
            default:
                break;

            } // switch on first character

        } // not eof

    // compute centre of gravity
    // note that very large files may have numerical problems with this
    centreOfGravity = Cartesian3(0.0, 0.0, 0.0);

    // if there are any vertices at all
    if (vertices.size() != 0)
        { // non-empty vertex set
        // sum up all of the vertex positions
        for (unsigned int vertex = 0; vertex < vertices.size(); vertex++)
            centreOfGravity = centreOfGravity + vertices[vertex];
        
        // and divide through by the number to get the average position
        // also known as the barycentre
        centreOfGravity = centreOfGravity / vertices.size();

        // start with 0 radius
        objectSize = 0.0;

        // now compute the largest distance from the origin to a vertex
        for (unsigned int vertex = 0; vertex < vertices.size(); vertex++)
            { // per vertex
            // compute the distance from the barycentre
            float distance = (vertices[vertex] - centreOfGravity).length();         
            
            // now test for maximality
            if (distance > objectSize)
                objectSize = distance;
                
            } // per vertex
        } // non-empty vertex set

    // return a success code
    return true;
	} // ReadObjectStream()

// write routine
void AttributedObject::WriteObjectStream(std::ostream &geometryStream)
    { // WriteObjectStream()
    geometryStream << "# " << (faceVertices.size()/3) << " triangles" << std::endl;
    geometryStream << std::endl;

    // output the vertex coordinates
    geometryStream << "# " << vertices.size() << " vertices" << std::endl;
    for (unsigned int vertex = 0; vertex < vertices.size(); vertex++)
        geometryStream << "v  " << std::fixed << vertices[vertex] << std::endl;

    // output the vertex colours
    geometryStream << "# " << colours.size() << " vertex colours" << std::endl;
    for (unsigned int vertex = 0; vertex < colours.size(); vertex++)
        geometryStream << "vc " << std::fixed << colours[vertex] << std::endl;

    // output the vertex normals
    geometryStream << "# " << normals.size() << " vertex normals" << std::endl;
    for (unsigned int vertex = 0; vertex < normals.size(); vertex++)
        geometryStream << "vn " << std::fixed << normals[vertex] << std::endl;

    // output the vertex colours
    geometryStream << "# " << textureCoords.size() << " vertex tex coords" << std::endl;
    for (unsigned int vertex = 0; vertex < textureCoords.size(); vertex++)
        geometryStream << "vt " << std::fixed << textureCoords[vertex] << std::endl;

    // and the faces
    for (unsigned int face = 0; face < faceVertices.size(); face+=3)
        { // per face
        geometryStream << "f";
        
        // loop through # of vertices
        for (unsigned int vertex = 0; vertex < 3; vertex++)
			{ // per vertex
            geometryStream << " ";
            geometryStream << faceVertices[face+vertex] + 1 << "/";
            geometryStream << faceColours[face+vertex] + 1 << "/";
            geometryStream << faceTexCoords[face+vertex] + 1 << "/";
            geometryStream << faceNormals[face+vertex] + 1;
			} // per vertex
		// end the line
        geometryStream << std::endl;
        } // per face
    
    } // WriteObjectStream()

// routine to render
void AttributedObject::Render(RenderParameters *renderParameters)
    { // Render()
	// make sure that textures are disabled
	glDisable(GL_TEXTURE_2D);

    // Scale defaults to the zoom setting
    float scale = renderParameters->zoomScale;
    scale /= objectSize;
    glTranslatef(-centreOfGravity.x * scale, -centreOfGravity.y * scale, -centreOfGravity.z * scale);

    // start rendering
    glBegin(GL_TRIANGLES);

    // loop through the faces: note that they may not be triangles, which complicates life
    for (unsigned int face = 0; face < faceVertices.size(); face+=3)
        { // per face
		// now do a loop over three vertices
		for (unsigned int vertex = 0; vertex < 3; vertex++)
			{ // per vertex
			glColor3f
				(
				colours[faceVertices[face+vertex]].x,
				colours[faceVertices[face+vertex]].y,
				colours[faceVertices[face+vertex]].z
				);

			glVertex3f
				(
				scale * vertices[faceVertices[face+vertex]].x,
				scale * vertices[faceVertices[face+vertex]].y,
				scale * vertices[faceVertices[face+vertex]].z
				);
			} // per vertex
        } // per face

    // close off the triangles
    glEnd();
    } // Render()

void AttributedObject::print()
{
    // Information of all variables
    std::cout << "Face Vertices: " << faceVertices.size() << "\n";
    std::cout << "Face Colours: " << faceColours.size() << "\n";
    std::cout << "Face Normals: " << faceNormals.size() << "\n";
    std::cout << "Face Tex Coords: " << faceTexCoords.size() << "\n";
    std::cout << "\n";
    std::cout << "Vertices: " << vertices.size() << "\n";
    std::cout << "Colour: " << colours.size() << "\n";
    std::cout << "Normals: " << normals.size() << "\n";
    std::cout << "Tex Coords: " << textureCoords.size() << "\n";
    std::cout << "\n";
    std::cout << "Center of gravity: " << centreOfGravity << "\n";
    std::cout << "\n";
}

void AttributedObject::outputTexture(std::string filename)
{
    std::string outputName = "output/" + filename + "_texture.ppm";

    std::ofstream outfile;
    outfile.open(outputName);

    // Initialise the uv map
    for(int height = 0; height <= 1024; height++)
    {
        std::vector<Cartesian3> mapCol;
        for(int width = 0; width <= 1024; width++)
        {
            Cartesian3 colour;
            colour.x = 0;
            colour.y = 0;
            colour.z = 0;
            mapCol.push_back(colour);
        }
        uvMap.push_back(mapCol);
    }

    for(int i = 0; i < faceVertices.size() / 3; i++)
    {
        // Get uv texture coordinate
        int v0 = (1 - textureCoords[faceTexCoords[i*3]].y) * 1024;
        int u0 = textureCoords[faceTexCoords[i*3]].x * 1024;

        int v1 = (1 - textureCoords[faceTexCoords[i*3+1]].y) * 1024;
        int u1 = textureCoords[faceTexCoords[i*3+1]].x * 1024;

        int v2 = (1 - textureCoords[faceTexCoords[i*3+2]].y) * 1024;
        int u2 = textureCoords[faceTexCoords[i*3+2]].x * 1024;

        // Map the mesh to uv coordinate
        // It is converted to int for outputing to ppm
        // As ppm files does not accept float values
        uvMap[v0][u0].x = (int) (colours[faceColours[i*3]].x * 255);
        uvMap[v0][u0].y = (int) (colours[faceColours[i*3]].y * 255);
        uvMap[v0][u0].z = (int) (colours[faceColours[i*3]].z * 255);

        uvMap[v1][u1].x = (int) (colours[faceColours[i*3+1]].x * 255);
        uvMap[v1][u1].y = (int) (colours[faceColours[i*3+1]].y * 255);
        uvMap[v1][u1].z = (int) (colours[faceColours[i*3+1]].z * 255);

        uvMap[v2][u2].x = (int) (colours[faceColours[i*3+2]].x * 255);
        uvMap[v2][u2].y = (int) (colours[faceColours[i*3+2]].y * 255);
        uvMap[v2][u2].z = (int) (colours[faceColours[i*3+2]].z * 255);

        // Draw triangle in our uv map using the three coordinates 
        // we calculated above
        drawTriangle(u0, v0, u1, v1, u2, v2);
    }
    
    outfile << "P3" << "\n";
    // Height and width
    outfile << "1024 1024" << "\n";
    // Maximum RGB value
    outfile << "255" << "\n";

    // print all values in the order from top to bottom, left to right
    for(int height = 0; height < 1024; height++)
    {
        for(int width = 0; width < 1024; width++)
        {
            outfile << uvMap[height][width] << "\n";
        }
    }

    outfile.close();
}

void AttributedObject::outputNormal(std::string filename)
{
    std::string outputName = "output/" + filename + "_normal.ppm";

    std::ofstream outfile;
    outfile.open(outputName);

    // Initialise the uv map
    for(int height = 0; height <= 1024; height++)
    {
        std::vector<Cartesian3> mapCol;
        for(int width = 0; width <= 1024; width++)
        {
            Cartesian3 colour;
            colour.x = 0;
            colour.y = 0;
            colour.z = 0;
            mapCol.push_back(colour);
        }
        uvMap.push_back(mapCol);
    }

    for(int i = 0; i < faceVertices.size() / 3; i++)
    {   
        // Get uv texture coordinate
        int v0 = (1 - textureCoords[faceTexCoords[i*3]].y) * 1024;
        int u0 = textureCoords[faceTexCoords[i*3]].x * 1024;

        int v1 = (1 - textureCoords[faceTexCoords[i*3+1]].y) * 1024;
        int u1 = textureCoords[faceTexCoords[i*3+1]].x * 1024;

        int v2 = (1 - textureCoords[faceTexCoords[i*3+2]].y) * 1024;
        int u2 = textureCoords[faceTexCoords[i*3+2]].x * 1024;

        // Since normal is in the range -1 to 1 and we need
        // to map it to RGB values from 0 to 255.
        // Therefore, -1 = 0 and 1 = 255 in RGB values
        // We also convert it back to integer value as
        // ppm file does not accept float values
        uvMap[v0][u0].x = (int) 128 + (normals[faceNormals[i*3]].x * 128);
        uvMap[v0][u0].y = (int) 128 + (normals[faceNormals[i*3]].y * 128);
        uvMap[v0][u0].z = (int) 128 + (normals[faceNormals[i*3]].z * 128);

        uvMap[v1][u1].x = (int) 128 + (normals[faceNormals[i*3+1]].x * 128);
        uvMap[v1][u1].y = (int) 128 + (normals[faceNormals[i*3+1]].y * 128);
        uvMap[v1][u1].z = (int) 128 + (normals[faceNormals[i*3+1]].z * 128);

        uvMap[v2][u2].x = (int) 128 + (normals[faceNormals[i*3+2]].x * 128);
        uvMap[v2][u2].y = (int) 128 + (normals[faceNormals[i*3+2]].y * 128);
        uvMap[v2][u2].z = (int) 128 + (normals[faceNormals[i*3+2]].z * 128);

        // Draw triangle in our uv map using the three coordinates 
        // we calculated above
        drawTriangle(u0, v0, u1, v1, u2, v2);
    }
    
    outfile << "P3" << "\n";
    // Height and width
    outfile << "1024 1024" << "\n";
    // Maximum RGB value
    outfile << "255" << "\n";

    // print all values in the order from top to bottom, left to right
    for(int height = 0; height < 1024; height++)
    {
        for(int width = 0; width < 1024; width++)
        {
            outfile << uvMap[height][width] << "\n";
        }
    }

    outfile.close();
}

// Function to draw triangles in uv coordinate
// Most of the code is referenced to COMP5812M
// Foundations of Modelling and Rendering's
// Assignment 1
void AttributedObject::drawTriangle(float x0, float y0,
                                    float x1, float y1,
                                    float x2, float y2)
{
    int minX = std::min({x0, x1, x2});
    int minY = std::min({y0, y1, y2});
    int maxX = std::max({x0, x1, x2});
    int maxY = std::max({y0, y1, y2});

    minX = std::max(minX, 0);
    minY = std::max(minY, 0);
    maxX = std::min(maxX, (int) (uvMap[0].size() - 1));
    maxY = std::min(maxY, (int) (uvMap.size() - 1));

    Cartesian3 vertex0(x0, y0, 0);
    Cartesian3 vertex1(x1, y1, 0);
    Cartesian3 vertex2(x2, y2, 0);

    // now for each side of the triangle, compute the line vectors
    Cartesian3 vector01 = vertex1 - vertex0;
    Cartesian3 vector12 = vertex2 - vertex1;
    Cartesian3 vector20 = vertex0 - vertex2;

    // now compute the line normal vectors
    Cartesian3 normal01(-vector01.y, vector01.x, 0.0);  
    Cartesian3 normal12(-vector12.y, vector12.x, 0.0);  
    Cartesian3 normal20(-vector20.y, vector20.x, 0.0);  

    // we don't need to normalise them, because the square roots will cancel out in the barycentric coordinates
    float lineConstant01 = normal01.dot(vertex0);
    float lineConstant12 = normal12.dot(vertex1);
    float lineConstant20 = normal20.dot(vertex2);

    // and compute the distance of each vertex from the opposing side
    float distance0 = normal12.dot(vertex0) - lineConstant12;
    float distance1 = normal20.dot(vertex1) - lineConstant20;
    float distance2 = normal01.dot(vertex2) - lineConstant01;

    if ((distance0 == 0) || (distance1 == 0) || (distance2 == 0))
        return; 

    int v = 0;
    int u = 0;
    for(v = minY; v <= maxY; v++)
    {
        if (v < 0) continue;
        if (v >= uvMap.size()) continue;
        for(u = minX; u <= maxX; u++)
        {
            if (u < 0) continue;
            if (u >= uvMap[v].size()) continue;

            Cartesian3 pixel(u, v, 0);

            float alpha = (normal12.dot(pixel) - lineConstant12) / distance0;           
            float beta = (normal20.dot(pixel) - lineConstant20) / distance1;            
            float gamma = (normal01.dot(pixel) - lineConstant01) / distance2;        

            // now perform the half-plane test
            if ((alpha < 0.0) || (beta < 0.0) || (gamma < 0.0))
                continue;

            uvMap[v][u] = uvMap[y0][x0] * alpha
                          + uvMap[y1][x1] * beta
                          + uvMap[y2][x2] * gamma;

            // Convert all values to integer
            uvMap[v][u].x = (int) uvMap[v][u].x;
            uvMap[v][u].y = (int) uvMap[v][u].y;
            uvMap[v][u].z = (int) uvMap[v][u].z;
        }
    }
}