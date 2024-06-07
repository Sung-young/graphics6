#include "Mesh.h"

void Mesh::gen_gl_buffers()
{
    glGenBuffers(1, &position_buffer_);
    glGenBuffers(1, &color_buffer_);
    glGenBuffers(1, &normal_buffer_);
}


void Mesh::update_tv_indices()
{
    // triangle-vertex indices
    tv_indices_.clear();
    for (unsigned int i = 0; i < pmesh_->mNumFaces; ++i) 
    {
        aiFace& ai_face = pmesh_->mFaces[i];
        assert(ai_face.mNumIndices >= 3);

        // convert a polygon to a triangle fan
        for (unsigned int idx = 0; idx < ai_face.mNumIndices - 2; ++idx)
        {
            tv_indices_.push_back(ai_face.mIndices[0]);
            tv_indices_.push_back(ai_face.mIndices[idx+1]);
            tv_indices_.push_back(ai_face.mIndices[idx+2]);
        }
    }
}

void Mesh::set_gl_position_buffer_()
{
    assert(pmesh_->HasPositions());

    std::vector<glm::vec3>  tv_positions;       // per triangle-vertex 3D position (size = 3 x #triangles)
   
    // TODO: for each triangle, set tv_positions
    for (unsigned int i = 0; i < tv_indices_.size(); ++i)
    {
        unsigned int vIndex = tv_indices_[i];
        tv_positions.push_back(glm::vec3(pmesh_->mVertices[vIndex].x, pmesh_->mVertices[vIndex].y, pmesh_->mVertices[vIndex].z));
    }

    glBindBuffer(GL_ARRAY_BUFFER, position_buffer_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*tv_positions.size(), &tv_positions[0], GL_STATIC_DRAW);
}

void Mesh::set_gl_color_buffer_(unsigned int cs_idx)
{
    assert(pmesh_->HasVertexColors(cs_idx));

    std::vector<glm::vec3>  tv_colors;       // per triangle-vertex 3D position (size = 3 x #triangles)

    // TODO: for each triangle, set tv_colors

    for (unsigned int i = 0; i < tv_indices_.size(); ++i)
    {
        unsigned int vIndex = tv_indices_[i];
        tv_colors.push_back(glm::vec3(pmesh_->mColors[cs_idx][vIndex].r, pmesh_->mColors[cs_idx][vIndex].g, pmesh_->mColors[cs_idx][vIndex].b));
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*tv_colors.size(), &tv_colors[0], GL_STATIC_DRAW);

    is_color_ = true;
}


void Mesh::set_gl_normal_buffer_(ShadingType shading_type)
{
    std::vector<glm::vec3>      tv_flat_normals;    // per triangle-vertex flat normal (size = 3 x #triangles)
    std::vector<glm::vec3>      tv_smooth_normals;  // per triangle-vertex smooth normal (size = 3 x #triangles)
    std::vector<glm::vec3>      v_smooth_normals;   // per-vertex 3D normal (size = #vertices)

    // init normals
    v_smooth_normals.resize(pmesh_->mNumVertices);
    for (std::size_t i = 0; i < v_smooth_normals.size(); ++i)
        v_smooth_normals[i] = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // TODO: compute per-triangle normal & 
    //       add_up to tv_flat_normals & v_smooth_normals
    tv_flat_normals.resize(tv_indices_.size());
    for (std::size_t i = 0; i < pmesh_->mNumFaces; ++i)
    {
        aiFace& ai_face = pmesh_->mFaces[i];
        assert(ai_face.mNumIndices >= 3);
        
        aiVector3D p0 = pmesh_->mVertices[ai_face.mIndices[0]];
        aiVector3D p1 = pmesh_->mVertices[ai_face.mIndices[1]];
        aiVector3D p2 = pmesh_->mVertices[ai_face.mIndices[2]];
        
        glm::vec3 pv0 = glm::vec3 (p0.x, p0.y, p0.z);
        glm::vec3 pv1 = glm::vec3 (p1.x, p1.y, p1.z);
        glm::vec3 pv2 = glm::vec3 (p2.x, p2.y, p2.z);
        
        glm::vec3 t_normal = glm::normalize(glm::cross(pv0 - pv1, pv2 - pv1));
        
        tv_flat_normals[i * 3 + 0] = t_normal;
        tv_flat_normals[i * 3 + 1] = t_normal;
        tv_flat_normals[i * 3 + 2] = t_normal;
        
        v_smooth_normals[ai_face.mIndices[0]] += t_normal;
        v_smooth_normals[ai_face.mIndices[1]] += t_normal;
        v_smooth_normals[ai_face.mIndices[2]] += t_normal;
    }
    
    // normalize v_smooth_normals
    for (std::size_t i = 0; i < v_smooth_normals.size(); ++i)
        v_smooth_normals[i] = glm::normalize(v_smooth_normals[i]);

    // if pmesh_ has per-vertex normals, then just use them.
    if (pmesh_->HasNormals())
        memcpy(&v_smooth_normals[0], &pmesh_->mNormals[0], sizeof(pmesh_->mNormals[0])*pmesh_->mNumVertices);
   
    // TODO: set tv_smooth_normals from v_smooth_normals
    tv_smooth_normals.resize(pmesh_->mNumFaces * 3);

    for (std::size_t i = 0; i < pmesh_->mNumFaces; ++i) 
    {
        aiFace& face = pmesh_->mFaces[i];
        assert(face.mNumIndices == 3);  // We assume that the mesh is triangulated

        tv_smooth_normals[i * 3 + 0] = glm::normalize(v_smooth_normals[face.mIndices[0]]);
        tv_smooth_normals[i * 3 + 1] = glm::normalize(v_smooth_normals[face.mIndices[1]]);
        tv_smooth_normals[i * 3 + 2] = glm::normalize(v_smooth_normals[face.mIndices[2]]);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer_);
   
    if (shading_type == kSmooth)
    {   
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*tv_smooth_normals.size(), &tv_smooth_normals[0], GL_STATIC_DRAW);
    }
    else //if (shading_type == kFlat)
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*tv_flat_normals.size(), &tv_flat_normals[0], GL_STATIC_DRAW);
    }
}

void Mesh::set_gl_buffers(ShadingType shading_type)
{
    set_gl_position_buffer_();
    if (pmesh_->HasVertexColors(0))
        set_gl_color_buffer_(0);
    set_gl_normal_buffer_(shading_type);
}


void Mesh::draw(int loc_a_position, int loc_a_normal)
{
    // TODO : draw a triangular mesh
    // 앞으로 언급하는 배열 버퍼(GL_ARRAY_BUFFER)는 position_buffer로 지정
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer_);
    // 버텍스 쉐이더의 attribute 중 a_position 부분 활성화
    glEnableVertexAttribArray(loc_a_position);
    // 현재 배열 버퍼에 있는 데이터를 버텍스 쉐이더 a_position에 해당하는 attribute와 연결
    glVertexAttribPointer(loc_a_position, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // 앞으로 언급하는 배열 버퍼(GL_ARRAY_BUFFER)는 color_buffer로 지정
    glBindBuffer(GL_ARRAY_BUFFER, normal_buffer_);
    // 버텍스 쉐이더의 attribute 중 a_color 부분 활성화
    glEnableVertexAttribArray(loc_a_normal);
    // 현재 배열 버퍼에 있는 데이터를 버텍스 쉐이더 a_color에 해당하는 attribute와 연결
    glVertexAttribPointer(loc_a_normal, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glDrawArrays(GL_TRIANGLES, 0, tv_indices_.size());
    
    // 정점 attribute 배열 비활성화
    glDisableVertexAttribArray(loc_a_position);
    glDisableVertexAttribArray(loc_a_normal);
}
    
void Mesh::print_info()
{
    std::cout << "print mesh info" << std::endl;

    std::cout << "num vertices " << pmesh_->mNumVertices << std::endl;
    for (unsigned int i = 0; i < pmesh_->mNumVertices; ++i)
    {
        aiVector3D vertex = pmesh_->mVertices[i];
        std::cout << "  vertex  (" << vertex.x << ", " << vertex.y << ", " << vertex.z << ")" << std::endl;

        if (pmesh_->mColors[0] != NULL)
        {
            aiColor4D color = pmesh_->mColors[0][i];
            std::cout << "  color  (" << color.r << ", " << color.g << ", " << color.b << ", " << color.a << ")" << std::endl;
        }
    }
}
