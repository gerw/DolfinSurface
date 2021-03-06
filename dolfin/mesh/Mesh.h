// Copyright (C) 2006-2012 Anders Logg
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
// Modified by Johan Hoffman 2007
// Modified by Magnus Vikstrøm 2007
// Modified by Garth N. Wells 2007-2013
// Modified by Niclas Jansson 2008
// Modified by Kristoffer Selim 2008
// Modified by Andre Massing 2009-2010
// Modified by Marie E. Rognes 2012
// Modified by Mikael Mortensen 2012
// Modified by Jan Blechta 2013
//
// First added:  2006-05-08
// Last changed: 2013-05-23

#ifndef __MESH_H
#define __MESH_H

#include <string>
#include <utility>
#include <boost/shared_ptr.hpp>

#include <dolfin/common/Variable.h>
#include <dolfin/common/Hierarchical.h>
#include <dolfin/intersection/IntersectionOperator.h>
#include <dolfin/ale/MeshDisplacement.h>
#include "MeshData.h"
#include "MeshDomains.h"
#include "MeshGeometry.h"
#include "MeshConnectivity.h"
#include "MeshTopology.h"

namespace dolfin
{
  class BoundaryMesh;
  class CellType;
  class CSGGeometry;
  class Expression;
  class GenericFunction;
  class LocalMeshData;
  class MeshEntity;
  class Point;
  class SubDomain;


  /// A _Mesh_ consists of a set of connected and numbered mesh entities.
  ///
  /// Both the representation and the interface are
  /// dimension-independent, but a concrete interface is also provided
  /// for standard named mesh entities:
  ///
  /// .. tabularcolumns:: |c|c|c|
  ///
  /// +--------+-----------+-------------+
  /// | Entity | Dimension | Codimension |
  /// +========+===========+=============+
  /// | Vertex |  0        |             |
  /// +--------+-----------+-------------+
  /// | Edge   |  1        |             |
  /// +--------+-----------+-------------+
  /// | Face   |  2        |             |
  /// +--------+-----------+-------------+
  /// | Facet  |           |      1      |
  /// +--------+-----------+-------------+
  /// | Cell   |           |      0      |
  /// +--------+-----------+-------------+
  ///
  /// When working with mesh iterators, all entities and connectivity
  /// are precomputed automatically the first time an iterator is
  /// created over any given topological dimension or connectivity.
  ///
  /// Note that for efficiency, only entities of dimension zero
  /// (vertices) and entities of the maximal dimension (cells) exist
  /// when creating a _Mesh_. Other entities must be explicitly created
  /// by calling init(). For example, all edges in a mesh may be
  /// created by a call to mesh.init(1). Similarly, connectivities
  /// such as all edges connected to a given vertex must also be
  /// explicitly created (in this case by a call to mesh.init(0, 1)).

  class Mesh : public Variable, public Hierarchical<Mesh>
  {
  public:

    /// Create empty mesh
    Mesh();

    /// Copy constructor.
    ///
    /// *Arguments*
    ///     mesh (_Mesh_)
    ///         Object to be copied.
    Mesh(const Mesh& mesh);

    /// Create mesh from data file.
    ///
    /// *Arguments*
    ///     filename (std::string)
    ///         Name of file to load.
    explicit Mesh(std::string filename);

    /// Create a distributed mesh from local (per process) data.
    ///
    /// *Arguments*
    ///     local_mesh_data (_LocalMeshData_)
    ///         Data from which to build the mesh.
    explicit Mesh(LocalMeshData& local_mesh_data);

    /// Create mesh defined by Constructive Solid Geometry (CSG)
    ///
    /// *Arguments*
    ///     geometry (_CSGGeometry_)
    ///         The CSG geometry
    ///     resolution (std::size_t)
    ///         An integer specifying the mesh resolution
    Mesh(const CSGGeometry& geometry, std::size_t resolution);

    /// Create mesh defined by Constructive Solid Geometry (CSG)
    ///
    /// *Arguments*
    ///     geometry (_CSGGeometry_)
    ///         The CSG geometry
    ///     resolution (std::size_t)
    ///         An integer specifying the mesh resolution
    Mesh(boost::shared_ptr<const CSGGeometry> geometry,
         std::size_t resolution);

    /// Destructor.
    ~Mesh();

    /// Assignment operator
    ///
    /// *Arguments*
    ///     mesh (_Mesh_)
    ///         Another _Mesh_ object.
    const Mesh& operator=(const Mesh& mesh);

    /// Get number of vertices in mesh.
    ///
    /// *Returns*
    ///     std::size_t
    ///         Number of vertices.
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    std::size_t num_vertices() const
    { return _topology.size(0); }

    /// Get number of edges in mesh.
    ///
    /// *Returns*
    ///     std::size_t
    ///         Number of edges.
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    std::size_t num_edges() const
    { return _topology.size(1); }

    /// Get number of faces in mesh.
    ///
    /// *Returns*
    ///     std::size_t
    ///         Number of faces.
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    std::size_t num_faces() const
    { return _topology.size(2); }

    /// Get number of facets in mesh.
    ///
    /// *Returns*
    ///     std::size_t
    ///         Number of facets.
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    std::size_t num_facets() const
    { return _topology.size(_topology.dim() - 1); }

    /// Get number of cells in mesh.
    ///
    /// *Returns*
    ///     std::size_t
    ///         Number of cells.
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    std::size_t num_cells() const
    { return _topology.size(_topology.dim()); }

    /// Get number of entities of given topological dimension.
    ///
    /// *Arguments*
    ///     d (std::size_t)
    ///         Topological dimension.
    ///
    /// *Returns*
    ///     std::size_t
    ///         Number of entities of topological dimension d.
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    std::size_t num_entities(std::size_t d) const
    { return _topology.size(d); }

    /// Get vertex coordinates.
    ///
    /// *Returns*
    ///     std::vector<double>&
    ///         Coordinates of all vertices.
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    std::vector<double>& coordinates()
    { return _geometry.x(); }

    /// Return coordinates of all vertices (const version).
    const std::vector<double>& coordinates() const
    { return _geometry.x(); }

    /// Get cell connectivity.
    ///
    /// *Returns*
    ///     std::vector<std::size_t>
    ///         Connectivity for all cells.
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    const std::vector<unsigned int>& cells() const
    { return _topology(_topology.dim(), 0)(); }

    /// Get number of local entities of given topological dimension.
    ///
    /// *Arguments*
    ///     dim (std::size_t)
    ///         Topological dimension.
    ///
    /// *Returns*
    ///     std::size_t
    ///         Number of local entities of topological dimension d.
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    std::size_t size(std::size_t dim) const
    { return _topology.size(dim); }

    /// Get global number of entities of given topological dimension.
    ///
    /// *Arguments*
    ///     dim (std::size_t)
    ///         Topological dimension.
    ///
    /// *Returns*
    ///     std::size_t
    ///         Global number of entities of topological dimension d.
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    std::size_t size_global(std::size_t dim) const
    { return _topology.size_global(dim); }

    /// Get mesh topology.
    ///
    /// *Returns*
    ///     _MeshTopology_
    ///         The topology object associated with the mesh.
    MeshTopology& topology()
    { return _topology; }

    /// Get mesh topology (const version).
    const MeshTopology& topology() const
    { return _topology; }

    /// Get mesh geometry.
    ///
    /// *Returns*
    ///     _MeshGeometry_
    ///         The geometry object associated with the mesh.
    MeshGeometry& geometry()
    { return _geometry; }

    /// Get mesh geometry (const version).
    const MeshGeometry& geometry() const
    { return _geometry; }

    /// Get mesh (sub)domains.
    ///
    /// *Returns*
    ///     _MeshDomains_
    ///         The (sub)domains associated with the mesh.
    MeshDomains& domains()
    { return _domains; }

    /// Get mesh (sub)domains.
    const MeshDomains& domains() const
    { return _domains; }

    /// Get intersection operator.
    ///
    /// *Returns*
    ///     _IntersectionOperator_
    ///         The intersection operator object associated with the mesh.
    IntersectionOperator& intersection_operator();

    /// Return intersection operator (const version);
    const IntersectionOperator& intersection_operator() const;

    /// Get mesh data.
    ///
    /// *Returns*
    ///     _MeshData_
    ///         The mesh data object associated with the mesh.
    MeshData& data();

    /// Get mesh data (const version).
    const MeshData& data() const;

    /// Get mesh cell type.
    ///
    /// *Returns*
    ///     _CellType_
    ///         The cell type object associated with the mesh.
    CellType& type()
    { dolfin_assert(_cell_type); return *_cell_type; }

    /// Get mesh cell type (const version).
    const CellType& type() const
    { dolfin_assert(_cell_type); return *_cell_type; }

    /// Compute entities of given topological dimension.
    ///
    /// *Arguments*
    ///     dim (std::size_t)
    ///         Topological dimension.
    ///
    /// *Returns*
    ///     std::size_t
    ///         Number of created entities.
    std::size_t init(std::size_t dim) const;

    /// Compute connectivity between given pair of dimensions.
    ///
    /// *Arguments*
    ///     d0 (std::size_t)
    ///         Topological dimension.
    ///
    ///     d1 (std::size_t)
    ///         Topological dimension.
    void init(std::size_t d0, std::size_t d1) const;

    /// Compute all entities and connectivity.
    void init() const;

    /// Clear all mesh data.
    void clear();

    /// Clean out all auxiliary topology data. This clears all
    /// topological data, except the connectivity between cells and
    /// vertices.
    void clean();

    /// Order all mesh entities.
    ///
    /// .. seealso::
    ///
    ///     UFC documentation (put link here!)
    void order();

    /// Check if mesh is ordered according to the UFC numbering convention.
    ///
    /// *Returns*
    ///     bool
    ///         The return values is true iff the mesh is ordered.
    bool ordered() const;

    /// Renumber mesh entities by coloring. This function is currently
    /// restricted to renumbering by cell coloring. The cells
    /// (cell-vertex connectivity) and the coordinates of the mesh are
    /// renumbered to improve the locality within each color. It is
    /// assumed that the mesh has already been colored and that only
    /// cell-vertex connectivity exists as part of the mesh.
    Mesh renumber_by_color() const;

    /// Rotate mesh around a coordinate axis through center of mass
    /// of all mesh vertices
    ///
    /// *Arguments*
    ///     angle (double)
    ///         The number of degrees (0-360) of rotation
    ///     axis (std::size_t)
    ///         The coordinate axis around which to rotate the mesh
    void rotate(double angle, std::size_t axis=2);

    /// Rotate mesh around a coordinate axis through a given point
    ///
    /// *Arguments*
    ///     angle (double)
    ///         The number of degrees (0-360) of rotation
    ///     axis (std::size_t)
    ///         The coordinate axis around which to rotate the mesh
    ///     point (_Point_)
    ///         The point around which to rotate the mesh
    void rotate(double angle, std::size_t axis, const Point& p);

    /// Move coordinates of mesh according to new boundary coordinates.
    ///
    /// *Arguments*
    ///     boundary (_BoundaryMesh_)
    ///         A mesh containing just the boundary cells.
    ///
    /// *Returns*
    ///     MeshDisplacement
    ///         Displacement encapsulated in Expression subclass
    ///         MeshDisplacement
    boost::shared_ptr<MeshDisplacement> move(BoundaryMesh& boundary);

    /// Move coordinates of mesh according to adjacent mesh with
    /// common global vertices.
    ///
    /// *Arguments*
    ///     mesh (_Mesh_)
    ///         A _Mesh_ object.
    ///
    /// *Returns*
    ///     MeshDisplacement
    ///         Displacement encapsulated in Expression subclass
    ///         MeshDisplacement
    boost::shared_ptr<MeshDisplacement> move(Mesh& mesh);

    /// Move coordinates of mesh according to displacement function.
    ///
    /// *Arguments*
    ///     displacement (_GenericFunction_)
    ///         A _GenericFunction_ object.
    void move(const GenericFunction& displacement);

    /// Smooth internal vertices of mesh by local averaging.
    ///
    /// *Arguments*
    ///     num_iterations (std::size_t)
    ///         Number of iterations to perform smoothing,
    ///         default value is 1.
    void smooth(std::size_t num_iterations=1);

    /// Smooth boundary vertices of mesh by local averaging.
    ///
    /// *Arguments*
    ///     num_iterations (std::size_t)
    ///         Number of iterations to perform smoothing,
    ///         default value is 1.
    ///
    ///     harmonic_smoothing (bool)
    ///         Flag to turn on harmonics smoothing, default
    ///         value is true.
    void smooth_boundary(std::size_t num_iterations=1,
                         bool harmonic_smoothing=true);

    /// Snap boundary vertices of mesh to match given sub domain.
    ///
    /// *Arguments*
    ///     sub_domain (_SubDomain_)
    ///         A _SubDomain_ object.
    ///
    ///     harmonic_smoothing (bool)
    ///         Flag to turn on harmonics smoothing, default
    ///         value is true.
    void snap_boundary(const SubDomain& sub_domain,
                       bool harmonic_smoothing=true);

    /// Color the cells of the mesh such that no two neighboring cells
    /// share the same color. A colored mesh keeps a
    /// CellFunction<std::size_t> named "cell colors" as mesh data which
    /// holds the colors of the mesh.
    ///
    /// *Arguments*
    ///     coloring_type (std::string)
    ///         Coloring type, specifying what relation makes two
    ///         cells neighbors, can be one of "vertex", "edge" or
    ///         "facet".
    ///
    /// *Returns*
    ///     std::vector<std::size_t>
    ///         The colors as a mesh function over the cells of the mesh.
    const std::vector<std::size_t>& color(std::string coloring_type) const;

    /// Color the cells of the mesh such that no two neighboring cells
    /// share the same color. A colored mesh keeps a
    /// CellFunction<std::size_t> named "cell colors" as mesh data which
    /// holds the colors of the mesh.
    ///
    /// *Arguments*
    ///     coloring_type (std::vector<std::size_t>)
    ///         Coloring type given as list of topological dimensions,
    ///         specifying what relation makes two mesh entinties neighbors.
    ///
    /// *Returns*
    ///     std::vector<std::size_t>
    ///         The colors as a mesh function over entities of the mesh.
    const std::vector<std::size_t>&
      color(std::vector<std::size_t> coloring_type) const;

    /// Compute all cells which are intersected by the given point.
    ///
    /// *Arguments*
    ///     point (_Point_)
    ///         A _Point_ object.
    ///
    ///     cells (std::set<std::size_t>)
    ///         A set of indices of all intersected cells.
    void intersected_cells(const Point& point,
                           std::set<std::size_t>& cells) const;

    /// Compute all cells which are intersected by any of a vector of points.
    ///
    /// *Arguments*
    ///     points (std::vector<_Point_>)
    ///         A vector of _Point_ objects.
    ///
    ///     cells (std::set<std::size_t>)
    ///         A set of indices of all intersected cells.
    void intersected_cells(const std::vector<Point>& points,
                           std::set<std::size_t>& cells) const;

    /// Compute all cells which are intersected by the given entity.
    ///
    /// *Arguments*
    ///     entity (_MeshEntity_)
    ///         A _MeshEntity_ object.
    ///
    ///     cells (std::vector<std::size_t>)
    ///         A vector of indices of all intersected cells.
    void intersected_cells(const MeshEntity& entity,
                           std::vector<std::size_t>& cells) const;

    /// Compute all cells which are intersected by any of a vector of entities.
    ///
    /// *Arguments*
    ///     entities (std::vector<_MeshEntity_>)
    ///         A vector of _MeshEntity_ objects.
    ///
    ///     cells (std::set<std::size_t>)
    ///         A vector of indices of all intersected cells.
    void intersected_cells(const std::vector<MeshEntity>& entities,
                           std::set<std::size_t>& cells) const;

    /// Compute all cells which are intersected by the given mesh.
    ///
    /// *Arguments*
    ///     mesh (_Mesh_)
    ///         A _Mesh_ object.
    ///
    ///     cells (std::set<std::size_t>)
    ///         A set of indices of all intersected cells.
    void intersected_cells(const Mesh& mesh,
                           std::set<std::size_t>& cells) const;

    /// Find the cell (if any) containing the given point. If the point
    /// is contained in several cells, the first cell is returned.
    ///
    /// *Arguments*
    ///     point (_Point_)
    ///         A _Point_ object.
    ///
    /// *Returns*
    ///     int
    ///         The index of the cell containing the point. If no cell
    ///         is found, the return value is -1.
    int intersected_cell(const Point& point) const;

    /// Find the point in the mesh closest to the given point.
    ///
    /// *Arguments*
    ///     point (_Point_)
    ///         A _Point_ object.
    ///
    /// *Returns*
    ///     _Point_
    ///         The closest point.
    Point closest_point(const Point& point) const;

    /// Find the cell in the mesh closest to the given point.
    ///
    /// *Arguments*
    ///     point (_Point_)
    ///         A _Point_ object.
    ///
    /// *Returns*
    ///     std::size_t
    ///         The index of the closest cell.
    ///
    /// *Example*
    ///     .. code-block:: c++
    ///
    ///         UnitSquare mesh(1, 1);
    ///         Point point(0.0, 2.0);
    ///         info("%d", mesh.closest_cell(point));
    ///
    ///     output::
    ///
    ///         1
    std::size_t closest_cell(const Point& point) const;

    /// Find the point and corresponding cell closest to the given point.
    ///
    /// *Arguments*
    ///     point (_Point_)
    ///         A _Point_ object.
    ///
    /// *Returns*
    ///     std::pair<_Point_, std::size_t>
    ///         A pair consisting of the closest point and corresponding
    ///         cell index.
    std::pair<Point, std::size_t>
      closest_point_and_cell(const Point& point) const;

    /// Computes the distance between a given point and the mesh
    ///
    /// *Arguments*
    ///     point (_Point_)
    ///         A _Point_ object.
    ///
    /// *Returns*
    ///     double
    ///         The distance to the mesh.
    double distance(const Point& point) const;

    /// Compute minimum cell diameter.
    ///
    /// *Returns*
    ///     double
    ///         The minimum cell diameter, the diameter is computed as
    ///         two times the circumradius
    ///         (http://mathworld.wolfram.com).
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    double hmin() const;

    /// Compute maximum cell diameter.
    ///
    /// *Returns*
    ///     double
    ///         The maximum cell diameter, the diameter is computed as
    ///         two times the circumradius
    ///         (http://mathworld.wolfram.com).
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    double hmax() const;

    /// Compute minimum cell inradius.
    ///
    /// *Returns*
    ///     double
    ///         The minimum of cells' inscribed sphere radii
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    double rmin() const;

    /// Compute maximum cell inradius.
    ///
    /// *Returns*
    ///     double
    ///         The maximum of cells' inscribed sphere radii
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    double rmax() const;

    /// Compute minimum normalized radius ratio of cells.
    ///
    /// *Returns*
    ///     double
    ///         The minimum over cells of normalized cell
    ///         radius ratio (which is = cell_dimension *
    ///         * inradius / circumradius; cell_dimension
    ///         is normalization factor).
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    double radius_ratio_min() const;

    /// Compute maximum normalized radius ratio of cells.
    ///
    /// *Returns*
    ///     double
    ///         The maximum over cells of normalized cell
    ///         radius ratio (which is = cell_dimension *
    ///         * inradius / circumradius; cell_dimension
    ///         is normalization factor).
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    double radius_ratio_max() const;

    /// Compute hash of mesh, currently based on the has of the mesh
    /// geometry and mesh topology.
    ///
    /// *Returns*
    ///     std::size_t
    ///         A tree-hashed value of the coordinates over all MPI processes
    ///
    std::size_t hash() const;

    /// Informal string representation.
    ///
    /// *Arguments*
    ///     verbose (bool)
    ///         Flag to turn on additional output.
    ///
    /// *Returns*
    ///     std::string
    ///         An informal representation of the mesh.
    ///
    /// *Example*
    ///     .. note::
    ///
    ///         No example code available for this function.
    std::string str(bool verbose) const;

    /// Return cell_orientations
    ///
    /// *Returns*
    ///     std::vector<int>
    ///         Map from cell index to orientation of cell
    std::vector<int>& cell_orientations();

    /// Return cell_orientations (const version)
    ///
    /// *Returns*
    ///     std::vector<int>
    ///         Map from cell index to orientation of cell
    const std::vector<int>& cell_orientations() const;

    /// Compute and initialize cell_orientations relative to a given
    /// global outward direction/normal/orientation. Only defined if
    /// mesh is orientable.
    ///
    /// *Arguments*
    ///     global_normal (Expression)
    ///         A global normal direction to the mesh
    void init_cell_orientations(const Expression& global_normal);

  private:

    // Friends
    friend class MeshEditor;
    friend class TopologyComputation;
    friend class MeshOrdering;
    friend class BinaryFile;

    // Mesh topology
    MeshTopology _topology;

    // Mesh geometry
    MeshGeometry _geometry;

    // Mesh domains
    MeshDomains _domains;

    // Auxiliary mesh data
    MeshData _data;

    // Cell type
    CellType* _cell_type;

    // Intersection detector
    IntersectionOperator _intersection_operator;

    // True if mesh has been ordered
    mutable bool _ordered;

    // Orientation of cells relative to a global direction
    std::vector<int> _cell_orientations;

  };
}

#endif
