#include <fbxsdk.h>

#include <cassert>

/* Tab character ("\t") counter */
int numTabs = 0; 

/**
 * Print the required number of tabs.
 */
void PrintTabs() {
    for(int i = 0; i < numTabs; i++)
        printf("   ");
}

/**
 * Return a string-based representation based on the attribute type.
 */
FbxString GetAttributeTypeName(FbxNodeAttribute::EType type) { 
    switch(type) { 
        case FbxNodeAttribute::eUnknown: return "unidentified"; 
        case FbxNodeAttribute::eNull: return "null"; 
        case FbxNodeAttribute::eMarker: return "marker"; 
        case FbxNodeAttribute::eSkeleton: return "skeleton"; 
        case FbxNodeAttribute::eMesh: return "mesh"; 
        case FbxNodeAttribute::eNurbs: return "nurbs"; 
        case FbxNodeAttribute::ePatch: return "patch"; 
        case FbxNodeAttribute::eCamera: return "camera"; 
        case FbxNodeAttribute::eCameraStereo: return "stereo"; 
        case FbxNodeAttribute::eCameraSwitcher: return "camera switcher"; 
        case FbxNodeAttribute::eLight: return "light"; 
        case FbxNodeAttribute::eOpticalReference: return "optical reference"; 
        case FbxNodeAttribute::eOpticalMarker: return "marker"; 
        case FbxNodeAttribute::eNurbsCurve: return "nurbs curve"; 
        case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface"; 
        case FbxNodeAttribute::eBoundary: return "boundary"; 
        case FbxNodeAttribute::eNurbsSurface: return "nurbs surface"; 
        case FbxNodeAttribute::eShape: return "shape"; 
        case FbxNodeAttribute::eLODGroup: return "lodgroup"; 
        case FbxNodeAttribute::eSubDiv: return "subdiv"; 
        default: return "unknown"; 
    } 
}

/**
 * Print an attribute.
 */
void PrintAttribute(FbxNodeAttribute* pAttribute) {
    if(!pAttribute) return;
 
    FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
    FbxString attrName = pAttribute->GetName();
    
    printf("\n");
    PrintTabs();
    switch(pAttribute->GetAttributeType()) {
        case FbxNodeAttribute::eMesh: {
            FbxMesh* mesh = static_cast<FbxMesh*>(pAttribute);
            if(mesh) {
                int fCount = mesh->GetPolygonCount();
                int* faces = mesh->GetPolygonVertices();
                
                printf("%s: %d faces ", typeName.Buffer(), fCount);
                printf("[");
                
                int offset = 0;
                for (int i = 0; i < fCount; i++) {
                    int vc = mesh->GetPolygonSize(i);
                    //assert(vc == 3);
                    for (int j = 0; j < vc; j++) {
                        printf("%d, ", faces[offset+j]);
                    }
                    offset += vc;
                }
                printf("]\n");
                
                PrintTabs();
                int vCount = mesh->GetControlPointsCount();
                FbxVector4* positions = mesh->GetControlPoints();

                printf("      %d points ", vCount);
                for (int i = 0; i < vCount; i++) {
                    FbxVector4 p = positions[i];
                    printf("(%f, %f, %f)", p[0], p[1], p[2]);
                }
                printf("\n");
                
                PrintTabs();
                FbxArray<FbxVector4> normals;
                if(mesh->GetPolygonVertexNormals(normals)) {
                    int nCount = normals.Size();
                    printf("      %d normals ", nCount);
                    for (int i = 0; i < nCount; i++) {
                        FbxVector4 p = normals[i];
                        printf("(%f, %f, %f)", p[0], p[1], p[2]);
                    }
                }
                printf("\n");
                
                int uvLayerCount = mesh->GetUVLayerCount();
                if(uvLayerCount) {
                    printf("%d uv layers", uvLayerCount);
                    PrintTabs();
                    FbxArray<FbxVector2> uvs;
                    if(mesh->GetPolygonVertexUVs("", uvs)) {
                        int uvCount = uvs.Size();
                        printf("      %d uvs ", uvCount);
                        for (int i = 0; i < uvCount; i++) {
                            FbxVector2 p = uvs[i];
                            printf("(%f, %f)", p[0], p[1]);
                        }
                    }
                }
            }
        }break;
        default: break;
    }

    printf("\n");
}

/**
 * Print a node, its attributes, and all its children recursively.
 */
void PrintNode(FbxNode* pNode) {
    PrintTabs();
    const char* nodeName = pNode->GetName();
    FbxDouble3 translation = pNode->LclTranslation.Get(); 
    FbxDouble3 rotation = pNode->LclRotation.Get(); 
    FbxDouble3 scaling = pNode->LclScaling.Get();

    // Print the contents of the node.
    printf("%s t=%f%f%f r=%f%f%f s=%f,%f,%f",
        nodeName, 
        translation[0], translation[1], translation[2],
        rotation[0], rotation[1], rotation[2],
        scaling[0], scaling[1], scaling[2]
    );
    numTabs++;

    // Print the node's attributes.
    for(int i = 0; i < pNode->GetNodeAttributeCount(); i++)
        PrintAttribute(pNode->GetNodeAttributeByIndex(i));

    // Recursively print the children.
    for(int j = 0; j < pNode->GetChildCount(); j++)
        PrintNode(pNode->GetChild(j));

    numTabs--;
    PrintTabs();
}

void extractAnimationStack(FbxScene *pScene) {
    int numStacks = pScene->GetSrcObjectCount(FBX_TYPE(FbxAnimStack));
}

/**
 * Main function - loads the hard-coded fbx file,
 * and prints its contents in an xml format to stdout.
 */
int main(int argc, char** argv) {

    // Change the following filename to a suitable filename value.
    const char* lFilename = "testanim.fbx";
    
    // Initialize the SDK manager. This object handles all our memory management.
    FbxManager* lSdkManager = FbxManager::Create();
    
    // Create the IO settings object.
    FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
    lSdkManager->SetIOSettings(ios);

    // Create an importer using the SDK manager.
    FbxImporter* lImporter = FbxImporter::Create(lSdkManager,"");
    
    // Use the first argument as the filename for the importer.
    if(!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings())) { 
        printf("Call to FbxImporter::Initialize() failed.\n"); 
        printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString()); 
        exit(-1); 
    }
    
    // Create a new scene so that it can be populated by the imported file.
    FbxScene* lScene = FbxScene::Create(lSdkManager,"myScene");

    // Import the contents of the file into the scene.
    lImporter->Import(lScene);

    // The file is imported; so get rid of the importer.
    lImporter->Destroy();
    
    // Print the nodes of the scene and their attributes recursively.
    // Note that we are not printing the root node because it should
    // not contain any attributes.
    FbxNode* lRootNode = lScene->GetRootNode();
    if(lRootNode) {
        for(int i = 0; i < lRootNode->GetChildCount(); i++) {
            PrintNode(lRootNode->GetChild(i));
        }
    }
    // Destroy the SDK manager and all the other objects it was handling.
    lSdkManager->Destroy();
    return 0;
}
