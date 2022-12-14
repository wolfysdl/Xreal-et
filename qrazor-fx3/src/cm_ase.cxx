/// ============================================================================
/*
Copyright (C) 2004 Robert Beckebans <trebor_7@users.sourceforge.net>
Please see the file "AUTHORS" for a list of contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/// ============================================================================


/// includes ===================================================================
// system -------------------------------------------------------------------
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility.hpp>
#include <boost/spirit/utility/lists.hpp>
#include <boost/spirit/utility/escape_char.hpp>

// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "cm_ase.h"

#include "common.h"
#include "vfs.h"
#include "files.h"


static cmodel_ase_c*	cm_ase_model = NULL;

static int		cm_ase_vertexes_num = 0;
static int		cm_ase_faces_num = 0;

std::vector<vec3_c>	cm_ase_vertexes;
std::vector<index_t>	cm_ase_indexes;

static float		cm_ase_float0;
static float		cm_ase_float1;
static float		cm_ase_float2;


static void	CM_ASE_Version(int version)
{
	if(version != ASE_VERSION)
		Com_Error(ERR_DROP, "CM_ASE_Version: wrong version number (%i should be %i)", version, ASE_VERSION);
}

static void	CM_ASE_UnknownMaterialOption(char const* begin, char const* end)
{
	//Com_Printf("unknown ASE material option '%s'\n", std::string(begin, end).c_str());
}

static void	CM_ASE_AddShader(char const* begin, char const* end)
{
}

static void	CM_ASE_Material(char const* begin, char const* end)
{	
//	Com_DPrintf("CM_ASE_Material()\n");
}

static void	CM_ASE_GeomObject(char const* begin, char const* end)
{	
//	Com_DPrintf("CM_ASE_GeomObject()\n");
}

static void	CM_ASE_NodeName(char const* begin, char const* end)
{	
//	Com_DPrintf("CM_ASE_NodeName(%s)\n", std::string(begin, end).c_str());
}

static void	CM_ASE_NewMesh(char const* begin, char const* end)
{
	cm_ase_vertexes.clear();
	cm_ase_indexes.clear();
	
//	Com_DPrintf("CM_ASE_NewMesh()\n");
}

static void	CM_ASE_MeshVertexList(char const* begin, char const* end)
{	
//	Com_DPrintf("CM_ASE_MeshVertexList()\n");
}

static void	CM_ASE_MeshVertex(int value)
{	
//	Com_DPrintf("CM_ASE_MeshVertex(%i)\n", value);
}

static void	CM_ASE_MeshFaceList(char const* begin, char const* end)
{	
//	Com_DPrintf("CM_ASE_MeshFaceList()\n");
}

static void	CM_ASE_MeshTVertList(char const* begin, char const* end)
{	
//	Com_DPrintf("CM_ASE_MeshTVertList()\n");
}

static void	CM_ASE_MeshTFaceList(char const* begin, char const* end)
{	
//	Com_DPrintf("CM_ASE_MeshTFaceList()\n");
}

static void	CM_ASE_MeshCVertList(char const* begin, char const* end)
{	
//	Com_DPrintf("CM_ASE_MeshCVertList()\n");
}

static void	CM_ASE_MeshCFaceList(char const* begin, char const* end)
{	
//	Com_DPrintf("CM_ASE_MeshCFaceList()\n");
}

static void	CM_ASE_MeshNormals(char const* begin, char const* end)
{	
//	Com_DPrintf("CM_ASE_MeshNormals()\n");
}

static void	CM_ASE_GetVertexesNum(int vertexes_num)
{
//	if(vertexes_num <= 0)
//		Com_Error(ERR_DROP, "CM_ASE_GetVertexesNum: mesh has invalid vertices number %i", vertexes_num);

	cm_ase_vertexes_num = vertexes_num;
}

static void	CM_ASE_CheckVertexesNum(char const)
{
	if(cm_ase_vertexes_num != (int)cm_ase_vertexes.size())
		Com_Error(ERR_DROP, "CM_ASE_CheckVertexesNum: mesh has bad vertices number %i != %i", cm_ase_vertexes_num, cm_ase_vertexes.size());
}


static void	CM_ASE_GetFacesNum(int faces_num)
{
//	if(faces_num <= 0)
//		Com_Error(ERR_DROP, "CM_ASE_GetFacesNum: mesh has invalid faces number %i", faces_num);

	cm_ase_faces_num = faces_num;
}

static void	CM_ASE_CheckIndexesNum(char const)
{
	if((cm_ase_faces_num * 3) != (int)cm_ase_indexes.size())
		Com_Error(ERR_DROP, "CM_ASE_CheckIndexesNum: mesh has bad indices number %i != %i", (cm_ase_faces_num * 3), cm_ase_indexes.size());
}


static void	CM_ASE_Float0(float f0)
{
	cm_ase_float0 = f0;
}

static void	CM_ASE_Float1(float f1)
{
	cm_ase_float1 = f1;
}

static void	CM_ASE_Float2(float f2)
{
	cm_ase_float2 = f2;
}

static void	CM_ASE_PushVertex(char const* begin, char const* end)
{
	cm_ase_vertexes.push_back(vec3_c(cm_ase_float0, cm_ase_float1, cm_ase_float2));
}

static void	CM_ASE_PushVertexIndex(int index)
{
	cm_ase_indexes.push_back(index);
}

static void	CM_ASE_AddMesh(char const begin)
{
	for(std::vector<vec3_c>::const_iterator ir = cm_ase_vertexes.begin(); ir != cm_ase_vertexes.end(); ++ir)
	{
		cm_ase_model->vertexes.push_back(*ir);
	}
	
	for(std::vector<index_t>::const_iterator ir = cm_ase_indexes.begin(); ir != cm_ase_indexes.end(); ++ir)
	{
		cm_ase_model->indexes.push_back(*ir);
	}
}



struct cm_ase_model_grammar_t : public boost::spirit::grammar<cm_ase_model_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(cm_ase_model_grammar_t const& self)
		{
			// start grammar definition
			restofline
				=	boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::eol_p | boost::spirit::ch_p('}'))]]
				;
				
			skip_restofline
				=	restofline
				;
				
			skip_block
				=	boost::spirit::ch_p('{') >>
					*skip_restofline >>
					boost::spirit::ch_p('}')
				;
				
			scene
				=	boost::spirit::str_p("*SCENE") >> skip_block
				;
				
			material_list
				=	boost::spirit::str_p("*MATERIAL_LIST") >> boost::spirit::ch_p('{') >>
					boost::spirit::str_p("*MATERIAL_COUNT") >> boost::spirit::int_p >>
					+material >> 
					boost::spirit::ch_p('}')
				;
				
			material
				=	boost::spirit::str_p("*MATERIAL")[&CM_ASE_Material] >> boost::spirit::int_p >> boost::spirit::ch_p('{') >>
					+(material_map_diffuse | material_submaterial | material_option) >>
					boost::spirit::ch_p('}')
				;
				
			material_option
				=	material_name	|
					restofline[&CM_ASE_UnknownMaterialOption]
				;
			
			material_name
				=	boost::spirit::str_p("*MATERIAL_NAME") >>
					boost::spirit::ch_p('\"') >>
					//boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')]][&CM_ASE_AddShader] >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')][&CM_ASE_AddShader] >>
					boost::spirit::ch_p('\"')
				;
				
			material_map_diffuse
				=	boost::spirit::str_p("*MAP_DIFFUSE") >> skip_block
				;
				
			material_submaterial
				=	boost::spirit::str_p("*SUBMATERIAL") >> boost::spirit::int_p >> 
					boost::spirit::ch_p('{') >>
					*(material_map_diffuse | skip_restofline) >>
					boost::spirit::ch_p('}')
				;
				
			geomobject
				=	boost::spirit::str_p("*GEOMOBJECT")[&CM_ASE_GeomObject] >> boost::spirit::ch_p('{') >>
					node_name >>
					!node_parent >>
					node_tm >>
					+mesh >>
					*skip_restofline >>
					boost::spirit::ch_p('}')
				;
				
			node_name
				=	boost::spirit::str_p("*NODE_NAME") >> 
					boost::spirit::ch_p('\"') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')][&CM_ASE_NodeName] >>
					boost::spirit::ch_p('\"')
				;
				
			node_parent
				=	boost::spirit::str_p("*NODE_PARENT") >>
					boost::spirit::ch_p('\"') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')] >>
					boost::spirit::ch_p('\"')
				;
				
			node_tm
				=	boost::spirit::str_p("*NODE_TM") >> skip_block
				;
				
			mesh
				= 	boost::spirit::str_p("*MESH")[&CM_ASE_NewMesh] >> boost::spirit::ch_p('{') >>
					boost::spirit::str_p("*TIMEVALUE") >> boost::spirit::int_p >>
					boost::spirit::str_p("*MESH_NUMVERTEX") >> boost::spirit::int_p[&CM_ASE_GetVertexesNum] >>
					boost::spirit::str_p("*MESH_NUMFACES") >> boost::spirit::int_p[&CM_ASE_GetFacesNum] >>
					mesh_vertex_list >>
					mesh_face_list >>
					!(boost::spirit::str_p("*MESH_NUMTVERTEX") >> boost::spirit::int_p) >>
					!mesh_tvertlist >>
					!(boost::spirit::str_p("*MESH_NUMTVFACES") >> boost::spirit::int_p) >>
					!mesh_tfacelist >>
					!(boost::spirit::str_p("*MESH_NUMCVERTEX") >> boost::spirit::int_p) >>
					!mesh_cvertlist >>
					!(boost::spirit::str_p("*MESH_NUMCVFACES") >> boost::spirit::int_p) >>
					!mesh_cfacelist >>
					!mesh_normals >>
					*skip_restofline >>
					boost::spirit::ch_p('}')[&CM_ASE_AddMesh]
				;
				
			mesh_vertex_list
				=	boost::spirit::str_p("*MESH_VERTEX_LIST")[&CM_ASE_MeshVertexList] >> boost::spirit::ch_p('{') >>
					*mesh_vertex[&CM_ASE_PushVertex] >>
					boost::spirit::ch_p('}')[&CM_ASE_CheckVertexesNum]
				;
				
			mesh_vertex
				=	boost::spirit::str_p("*MESH_VERTEX") >> 
					boost::spirit::int_p[&CM_ASE_MeshVertex] >> 
					boost::spirit::real_p[&CM_ASE_Float0] >>
					boost::spirit::real_p[&CM_ASE_Float1] >>
					boost::spirit::real_p[&CM_ASE_Float2]
				;
				
			mesh_face_list
				=	boost::spirit::str_p("*MESH_FACE_LIST")[&CM_ASE_MeshFaceList] >> boost::spirit::ch_p('{') >>
					*mesh_face >>
					boost::spirit::ch_p('}')[&CM_ASE_CheckIndexesNum]
				;
				
			mesh_face
				=	boost::spirit::str_p("*MESH_FACE") >>
					boost::spirit::int_p >> boost::spirit::ch_p(':') >>
					boost::spirit::str_p("A:") >> boost::spirit::int_p[&CM_ASE_PushVertexIndex] >>
					boost::spirit::str_p("B:") >> boost::spirit::int_p[&CM_ASE_PushVertexIndex] >>
					boost::spirit::str_p("C:") >> boost::spirit::int_p[&CM_ASE_PushVertexIndex] >>
					skip_restofline
				;
				
			mesh_tvertlist
				=	boost::spirit::str_p("*MESH_TVERTLIST")[&CM_ASE_MeshTVertList] >> boost::spirit::ch_p('{') >>
					+mesh_tvert >>
					boost::spirit::ch_p('}')
				;
				
			mesh_tvert
				=	boost::spirit::str_p("*MESH_TVERT") >>
					boost::spirit::int_p >>
					boost::spirit::real_p[&CM_ASE_Float0] >>
					boost::spirit::real_p[&CM_ASE_Float1] >>
					boost::spirit::real_p[&CM_ASE_Float2]
				;
				
			mesh_tfacelist
				=	boost::spirit::str_p("*MESH_TFACELIST")[&CM_ASE_MeshTFaceList] >> boost::spirit::ch_p('{') >>
					+mesh_tface >>
					boost::spirit::ch_p('}')
				;
				
			mesh_tface
				=	boost::spirit::str_p("*MESH_TFACE") >>
					boost::spirit::int_p >> 
					boost::spirit::int_p >>
					boost::spirit::int_p >>
					boost::spirit::int_p
				;
				
			mesh_cvertlist
				=	boost::spirit::str_p("*MESH_CVERTLIST")[&CM_ASE_MeshCVertList] >> boost::spirit::ch_p('{') >>
					+mesh_vertcol >>
					boost::spirit::ch_p('}')
				;
				
			mesh_vertcol
				=	boost::spirit::str_p("*MESH_VERTCOL") >>
					boost::spirit::int_p >>
					boost::spirit::real_p >>
					boost::spirit::real_p >>
					boost::spirit::real_p
				;
				
			mesh_cfacelist
				=	boost::spirit::str_p("*MESH_CFACELIST")[&CM_ASE_MeshCFaceList] >> boost::spirit::ch_p('{') >>
					+mesh_cface >>
					boost::spirit::ch_p('}')
				;
				
			mesh_cface
				=	boost::spirit::str_p("*MESH_CFACE") >>
					boost::spirit::int_p >> 
					boost::spirit::int_p >>
					boost::spirit::int_p >>
					boost::spirit::int_p
				;
				
			mesh_normals
				=	boost::spirit::str_p("*MESH_NORMALS")[&CM_ASE_MeshNormals] >> boost::spirit::ch_p('{') >>
					*(	mesh_facenormal >> 
						mesh_vertexnormal >> 
						mesh_vertexnormal >>
						mesh_vertexnormal
					) >>
					boost::spirit::ch_p('}')
				;
				
			mesh_facenormal
				=	boost::spirit::str_p("*MESH_FACENORMAL") >>
					boost::spirit::int_p >>
					boost::spirit::real_p >>
					boost::spirit::real_p >>
					boost::spirit::real_p
				;
				
			mesh_vertexnormal
				=	boost::spirit::str_p("*MESH_VERTEXNORMAL") >>
					boost::spirit::int_p >>
					boost::spirit::real_p >>
					boost::spirit::real_p >>
					boost::spirit::real_p
				;
				
			expression
				=	boost::spirit::str_p("*3DSMAX_ASCIIEXPORT") >> boost::spirit::int_p[&CM_ASE_Version] >>
					boost::spirit::str_p("*COMMENT") >> restofline >>
					scene >>
					material_list >>
					+geomobject// >>
					//*boost::spirit::anychar_p
				;
				
			// end grammar definiton
		}
		
		boost::spirit::rule<ScannerT>	restofline,
						skip_restofline,
						skip_block,
						
						scene,
						material_list,
							material,
								material_option,
									material_name,
								material_map_diffuse,
								material_submaterial,
						geomobject,
							node_name,
							node_parent,
							node_tm,
							mesh,
								mesh_vertex_list,
									mesh_vertex,
								mesh_face_list,
									mesh_face,
								mesh_tvertlist,
									mesh_tvert,
								mesh_tfacelist,
									mesh_tface,
								mesh_cvertlist,
									mesh_vertcol,
								mesh_cfacelist,
									mesh_cface,
								mesh_normals,
									mesh_facenormal,
									mesh_vertexnormal,
						expression;
		
		boost::spirit::rule<ScannerT> const&
		start() const { return expression; }
	};
};


cmodel_ase_c::cmodel_ase_c(const std::string &name, byte *buffer, uint_t buffer_size)
:cmodel_c(name, buffer, buffer_size)
{
}

void	cmodel_ase_c::load()
{
	cm_ase_model = this;

	cm_ase_model_grammar_t grammar;

	boost::spirit::parse_info<> info = boost::spirit::parse((const char*)_buffer, grammar, boost::spirit::space_p);
	
	if(!info.full)
		Com_Error(ERR_DROP, "cm_ase_model_c::load: parsing failed for '%s'", getName());
		
	_aabb.clear();
	for(std::vector<vec3_c>::const_iterator ir = vertexes.begin(); ir != vertexes.end(); ++ir)
	{
		_aabb.addPoint(*ir);
	}
}

