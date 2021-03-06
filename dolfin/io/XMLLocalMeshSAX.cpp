// Copyright (C) 2008-2011 Ola Skavhaug and Garth N. Wells
//
// This file is part of DOLFIN.
//
// DOLFIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DOLFIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.
//
// Modified by Anders Logg 2008-2011
//
// First added:  2008-11-28
// Last changed: 2012-11-26

#include <iostream>

#include <map>
#include <utility>
#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/scoped_ptr.hpp>

#include <dolfin/common/constants.h>
#include <dolfin/common/MPI.h>
#include <dolfin/log/log.h>
#include <dolfin/mesh/CellType.h>
#include <dolfin/mesh/LocalMeshData.h>
#include "SAX2AttributeParser.h"
#include "XMLLocalMeshSAX.h"

using namespace dolfin;

//-----------------------------------------------------------------------------
XMLLocalMeshSAX::XMLLocalMeshSAX(LocalMeshData& mesh_data,
  const std::string filename) : state(OUTSIDE), _mesh_data(mesh_data),
  _filename(filename)
{
  // Do nothing
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::read()
{
  // Clear mesh data
  _mesh_data.clear();

  // Create SAX2 handler
  xmlSAXHandler sax_handler;
  memset(&sax_handler, 0, sizeof(sax_handler));
  sax_handler.initialized = XML_SAX2_MAGIC;

  // Call back functions
  sax_handler.startDocument = XMLLocalMeshSAX::sax_start_document;
  sax_handler.endDocument   = XMLLocalMeshSAX::sax_end_document;

  sax_handler.startElementNs = XMLLocalMeshSAX::sax_start_element;
  sax_handler.endElementNs   = XMLLocalMeshSAX::sax_end_element;

  sax_handler.warning = XMLLocalMeshSAX::sax_warning;
  sax_handler.error = XMLLocalMeshSAX::sax_error;

  // Parse file
  int err = xmlSAXUserParseFile(&sax_handler, (void *) this, _filename.c_str());
  if (err != 0)
  {
    dolfin_error("XMLLocalMeshSAX.cpp",
                 "read local mesh data",
                 "Error encountered by libxml2 when parsing XML file \"%s\"",
                 _filename.c_str());
  }
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::start_element(const xmlChar* name, const xmlChar** attrs,
                                    std::size_t num_attributes)
{
  switch (state)
  {
  case OUTSIDE:
    if (xmlStrcasecmp(name, (xmlChar* ) "mesh") == 0)
    {
      read_mesh(name, attrs, num_attributes);
      state = INSIDE_MESH;
    }
    break;

  case INSIDE_MESH:
    if (xmlStrcasecmp(name, (xmlChar* ) "vertices") == 0)
    {
      read_vertices(name, attrs, num_attributes);
      state = INSIDE_VERTICES;
    }
    else if (xmlStrcasecmp(name, (xmlChar* ) "cells") == 0)
    {
      read_cells(name, attrs, num_attributes);
      state = INSIDE_CELLS;
    }
    else if (xmlStrcasecmp(name, (xmlChar* ) "data") == 0)
    {
      //dolfin_error("XMLLocalMeshSAX.cpp",
      //             "read mesh data in parallel",
      //             "not implemented");
      //read_mesh_data(name, attrs, num_attributes);
      state = INSIDE_DATA;
    }
    else if (xmlStrcasecmp(name, (xmlChar* ) "domains") == 0)
    {
      state = INSIDE_DOMAINS;
    }
    break;

  case INSIDE_VERTICES:
    if (xmlStrcasecmp(name, (xmlChar* ) "vertex") == 0)
    {
      read_vertex(name, attrs, num_attributes);
    }
    break;

  case INSIDE_CELLS:
    if (xmlStrcasecmp(name, (xmlChar* ) "interval") == 0)
    {
      read_interval(name, attrs, num_attributes);
    }
    else if (xmlStrcasecmp(name, (xmlChar* ) "triangle") == 0)
    {
      read_triangle(name, attrs, num_attributes);
    }
    else if (xmlStrcasecmp(name, (xmlChar* ) "tetrahedron") == 0)
    {
      read_tetrahedron(name, attrs, num_attributes);
    }
    break;

  case INSIDE_DATA:
    if (xmlStrcasecmp(name, (xmlChar* ) "meshfunction") == 0)
    {
      //read_mesh_function(name, attrs);
      state = INSIDE_MESH_FUNCTION;
    }
    else if (xmlStrcasecmp(name, (xmlChar* ) "array") == 0)
    {
      //read_array(name, attrs);
      state = INSIDE_ARRAY;
    }
    else if (xmlStrcasecmp(name, (xmlChar* ) "data_entry") == 0)
    {
      //read_data_entry(name, attrs);
      state = INSIDE_DATA_ENTRY;
    }
    break;

  case INSIDE_DOMAINS:
    if (xmlStrcasecmp(name, (xmlChar* ) "mesh_value_collection") == 0)
    {
      read_mesh_value_collection(name, attrs, num_attributes);
      state = INSIDE_MESH_VALUE_COLLECTION;
    }
    break;

  case INSIDE_MESH_VALUE_COLLECTION:
    if (xmlStrcasecmp(name, (xmlChar* ) "value") == 0)
    {
      read_mesh_value_collection_entry(name, attrs, num_attributes);
      state = INSIDE_MESH_VALUE_COLLECTION;
    }
    break;

  case INSIDE_DATA_ENTRY:
    if (xmlStrcasecmp(name, (xmlChar* ) "array") == 0)
    {
      //read_array(name, attrs);
      state = INSIDE_ARRAY;
    }
    break;
  case DONE:
    dolfin_error("XMLLocalMeshSAX.cpp",
                 "read local mesh data",
                 "Inconsistent state in XML reader (%d). End of file reached", state);

  default:
    dolfin_error("XMLLocalMeshSAX.cpp",
                 "read local mesh data",
                 "Inconsistent state in XML reader (%d)", state);
  }
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::end_element(const xmlChar *name)
{
  switch (state)
  {
  case INSIDE_MESH:
    if (xmlStrcasecmp(name, (xmlChar* ) "mesh") == 0)
      state = DONE;
    break;

  case INSIDE_VERTICES:
    if (xmlStrcasecmp(name, (xmlChar* ) "vertices") == 0)
      state = INSIDE_MESH;
    break;

  case INSIDE_CELLS:
    if (xmlStrcasecmp(name, (xmlChar* ) "cells") == 0)
      state = INSIDE_MESH;
    break;

  case INSIDE_DATA:
    if (xmlStrcasecmp(name, (xmlChar* ) "data") == 0)
      state = INSIDE_MESH;
    break;

  case INSIDE_DOMAINS:
    if (xmlStrcasecmp(name, (xmlChar* ) "domains") == 0)
      state = INSIDE_MESH;
    break;

  case INSIDE_MESH_VALUE_COLLECTION:
    if (xmlStrcasecmp(name, (xmlChar* ) "mesh_value_collection") == 0)
      state = INSIDE_DOMAINS;
    break;

  case INSIDE_MESH_FUNCTION:
    if (xmlStrcasecmp(name, (xmlChar* ) "meshfunction") == 0)
      state = INSIDE_DATA;
    break;

  case INSIDE_DATA_ENTRY:
    if (xmlStrcasecmp(name, (xmlChar* ) "data_entry") == 0)
      state = INSIDE_DATA;

  case INSIDE_ARRAY:
    if (xmlStrcasecmp(name, (xmlChar* ) "array") == 0)
      state = INSIDE_DATA_ENTRY;
    if (xmlStrcasecmp(name, (xmlChar* ) "data_entry") == 0)
      state = INSIDE_DATA;
    break;

  default:
    {
     //warning("Closing XML tag '%s', but state is %d.", name, state);
    }
  }
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::sax_start_document(void *ctx)
{
  // Do nothing
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::sax_end_document(void *ctx)
{
  // Do nothing
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::sax_start_element(void* ctx,
                                                    const xmlChar* name,
                                                    const xmlChar* prefix,
                                                    const xmlChar* URI,
                                                    int nb_namespaces,
                                                    const xmlChar** namespaces,
                                                    int nb_attributes,
                                                    int nb_defaulted,
                                                    const xmlChar** attrs)
{
  ((XMLLocalMeshSAX*) ctx)->start_element(name, attrs, nb_attributes);
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::sax_end_element(void* ctx,
                                                  const xmlChar* name,
                                                  const xmlChar* prefix,
                                                  const xmlChar* URI)
{
  ((XMLLocalMeshSAX*) ctx)->end_element(name);
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::sax_warning(void *ctx, const char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  char buffer[DOLFIN_LINELENGTH];
  vsnprintf(buffer, DOLFIN_LINELENGTH, msg, args);
  warning("Incomplete XML data: " + std::string(buffer));
  va_end(args);
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::sax_error(void *ctx, const char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  char buffer[DOLFIN_LINELENGTH];
  vsnprintf(buffer, DOLFIN_LINELENGTH, msg, args);
  dolfin_error("XMLLocalMeshSAX.cpp",
               "read local mesh data",
               "Illegal XML data (\"%s\")",
               std::string(buffer).c_str());
  va_end(args);
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::sax_fatal_error(void *ctx, const char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  char buffer[DOLFIN_LINELENGTH];
  vsnprintf(buffer, DOLFIN_LINELENGTH, msg, args);
  dolfin_error("XMLLocalMeshSAX.cpp",
               "read local mesh data",
               "Illegal XML data: (\"%s\")",
               std::string(buffer).c_str());
  va_end(args);
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::read_mesh(const xmlChar* name, const xmlChar** attrs,
                                std::size_t num_attributes)
{
  // Parse values
  std::string type = SAX2AttributeParser::parse<std::string>(name, attrs, "celltype", num_attributes);
  gdim = SAX2AttributeParser::parse<unsigned int>(name, attrs, "dim", num_attributes);

  // Create cell type to get topological dimension
  boost::scoped_ptr<CellType> cell_type(CellType::create(type));
  tdim = cell_type->dim();

  // Get number of entities for topological dimension 0
  _mesh_data.tdim = tdim;
  _mesh_data.gdim = gdim;
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::read_vertices(const xmlChar* name, const xmlChar** attrs,
                                    std::size_t num_attributes)
{
  // Parse the number of global vertices
  const std::size_t num_global_vertices = SAX2AttributeParser::parse<unsigned int>(name, attrs, "size", num_attributes);
  _mesh_data.num_global_vertices = num_global_vertices;

  // Compute vertex range
  vertex_range = MPI::local_range(num_global_vertices);

  // Reserve space for local-to-global vertex map and vertex coordinates
  _mesh_data.vertex_indices.reserve(num_local_vertices());
  _mesh_data.vertex_coordinates.resize(boost::extents[num_local_vertices()][_mesh_data.gdim]);
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::read_vertex(const xmlChar* name, const xmlChar** attrs,
                                  std::size_t num_attributes)
{
  // Read vertex index
  const std::size_t v = SAX2AttributeParser::parse<unsigned int>(name, attrs, "index", num_attributes);

  // Skip vertices not in range for this process
  if (v < vertex_range.first || v >= vertex_range.second)
    return;

  const std::size_t local_index = v - vertex_range.first;
  const char *xyz[] = {"x", "y", "z"};

  for(std::size_t i = 0; i < gdim ; ++i)
    _mesh_data.vertex_coordinates[local_index][i] = SAX2AttributeParser::parse<double>(name, attrs, xyz[i], num_attributes);

  // Store global vertex numbering
  _mesh_data.vertex_indices.push_back(v);
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::read_cells(const xmlChar* name, const xmlChar** attrs,
                                 std::size_t num_attributes)
{
  // Parse the number of global cells
  const std::size_t num_global_cells = SAX2AttributeParser::parse<unsigned int>(name, attrs, "size", num_attributes);
  _mesh_data.num_global_cells = num_global_cells;

  // Compute cell range
  cell_range = MPI::local_range(num_global_cells);

  // Allocate space for cells
  _mesh_data.cell_vertices.resize(boost::extents[num_local_cells()][_mesh_data.tdim + 1]);

  // Reserve space for global cell indices
  _mesh_data.global_cell_indices.reserve(num_local_cells());
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::read_interval(const xmlChar* name, const xmlChar** attrs,
                                    std::size_t num_attributes)
{
  // Check dimension
  if (tdim != 1)
  {
    dolfin_error("XMLLocalMeshSAX.cpp",
                 "read local mesh data",
                 "Mesh entity (interval) does not match dimension of mesh (%d)", tdim);
  }

  // Read cell index
  const std::size_t c = SAX2AttributeParser::parse<unsigned int>(name, attrs, "index", num_attributes);

  // Skip cells not in range for this process
  if (c < cell_range.first || c >= cell_range.second)
    return;

  // Add cell
  boost::multi_array<std::size_t , 2>::subarray<1>::type cell
      = _mesh_data.cell_vertices[c - cell_range.first];
  cell[0] = SAX2AttributeParser::parse<unsigned int>(name, attrs, "v0", num_attributes);
  cell[1] = SAX2AttributeParser::parse<unsigned int>(name, attrs, "v1", num_attributes);

  // Add global cell index
  _mesh_data.global_cell_indices.push_back(c);

  // Vertices per cell
  _mesh_data.num_vertices_per_cell = 2;
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::read_triangle(const xmlChar *name,
                                    const xmlChar **attrs,
                                    std::size_t num_attributes)
{
  // Check dimension
  if (tdim != 2)
  {
    dolfin_error("XMLLocalMeshSAX.cpp",
                 "read local mesh data",
                 "Mesh entity (interval) does not match dimension of mesh (%d)", tdim);
  }

  // Read cell index
  const std::size_t c = SAX2AttributeParser::parse<unsigned int>(name, attrs, "index", num_attributes);

  // Skip cells not in range for this process
  if (c < cell_range.first || c >= cell_range.second)
    return;

  // Add cell
  boost::multi_array<std::size_t, 2>::subarray<1>::type cell
    = _mesh_data.cell_vertices[c- cell_range.first];
  cell[0] = SAX2AttributeParser::parse<unsigned int>(name, attrs, "v0", num_attributes);
  cell[1] = SAX2AttributeParser::parse<unsigned int>(name, attrs, "v1", num_attributes);
  cell[2] = SAX2AttributeParser::parse<unsigned int>(name, attrs, "v2", num_attributes);

  // Add global cell index
  _mesh_data.global_cell_indices.push_back(c);

  // Vertices per cell
  _mesh_data.num_vertices_per_cell = 3;
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::read_tetrahedron(const xmlChar *name,
                                       const xmlChar **attrs,
                                       std::size_t num_attributes)
{
  // Check dimension
  if (tdim != 3)
  {
    dolfin_error("XMLLocalMeshSAX.cpp",
                 "read local mesh data",
                 "Mesh entity (interval) does not match dimension of mesh (%d)", tdim);
  }

  // Read cell index
  const std::size_t c = SAX2AttributeParser::parse<unsigned int>(name, attrs, "index", num_attributes);

  // Skip cells not in range for this process
  if (c < cell_range.first || c >= cell_range.second)
    return;

  // Add cell
  boost::multi_array<std::size_t, 2>::subarray<1>::type cell
    = _mesh_data.cell_vertices[c - cell_range.first];
  cell[0] = SAX2AttributeParser::parse<unsigned int>(name, attrs, "v0", num_attributes);
  cell[1] = SAX2AttributeParser::parse<unsigned int>(name, attrs, "v1", num_attributes);
  cell[2] = SAX2AttributeParser::parse<unsigned int>(name, attrs, "v2", num_attributes);
  cell[3] = SAX2AttributeParser::parse<unsigned int>(name, attrs, "v3", num_attributes);

  // Add global cell index
  _mesh_data.global_cell_indices.push_back(c);

  // Vertices per cell
  _mesh_data.num_vertices_per_cell = 4;
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::read_mesh_value_collection(const xmlChar* name,
                                                 const xmlChar** attrs,
                                                 std::size_t num_attributes)
{
  // Parse values
  const std::string type = SAX2AttributeParser::parse<std::string>(name, attrs, "type", num_attributes);
  const std::size_t dim = SAX2AttributeParser::parse<unsigned int>(name, attrs, "dim", num_attributes);
  const std::size_t size = SAX2AttributeParser::parse<unsigned int>(name, attrs, "size", num_attributes);

  // Compute domain value range
  domain_value_range = MPI::local_range(size);
  domain_dim = dim;

  if (type != "uint")
  {
    dolfin_error("XMLLocalMeshSAX.cpp",
                 "read local mesh data",
                 "XMLLocalMeshSAX can only read unsigned integer domain values");
  }

  _mesh_data.domain_data.insert(std::make_pair(dim, 0));

  // Reset counter
  domain_value_counter = 0;
}
//-----------------------------------------------------------------------------
void XMLLocalMeshSAX::read_mesh_value_collection_entry(const xmlChar* name,
                                                       const xmlChar** attrs,
                                                       std::size_t num_attributes)
{
  if (domain_value_counter >= domain_value_range.first && domain_value_counter < domain_value_range.second)
  {
    // Parse values
    std::pair<std::pair<std::size_t, unsigned int>, unsigned int> entry_data;
    entry_data.first.first  = SAX2AttributeParser::parse<unsigned int>(name, attrs, "cell_index", num_attributes);
    entry_data.first.second = SAX2AttributeParser::parse<unsigned int>(name, attrs, "local_entity", num_attributes);
    entry_data.second       = SAX2AttributeParser::parse<unsigned int>(name, attrs, "value", num_attributes);

    std::vector<std::pair<std::pair<std::size_t, std::size_t>, std::size_t> >& data
      = _mesh_data.domain_data.find(domain_dim)->second;
    data.push_back(entry_data);
  }

  ++domain_value_counter;
}
//-----------------------------------------------------------------------------
std::size_t XMLLocalMeshSAX::num_local_vertices() const
{
  return vertex_range.second - vertex_range.first;
}
//-----------------------------------------------------------------------------
std::size_t XMLLocalMeshSAX::num_local_cells() const
{
  return cell_range.second - cell_range.first;
}
//-----------------------------------------------------------------------------
