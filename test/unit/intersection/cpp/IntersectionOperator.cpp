// Copyright (C) 2011  André Massing
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
// Modified by André Massing, 2011
//
// First added:  2011-10-04
// Last changed: 2013-04-23
//
// Unit test for the IntersectionOperator

#include <dolfin.h>
#include <dolfin/common/unittest.h>

#include <vector>
#include <algorithm>

using namespace dolfin;

  template <std::size_t dim0, std::size_t dim1>
  void testEntityEntityIntersection(const Mesh& mesh)
  {
    //Compute incidences
    mesh.init(dim0, dim1);
    mesh.init(dim1, dim0);
    mesh.init(0, dim0);

    std::size_t label = 1;
    // Default is to mark all entities
    MeshFunction<std::size_t> labels(mesh, dim0, label);
    IntersectionOperator io(labels, label, "ExactPredicates");

    // Iterator over all entities and compute self-intersection
    // Should be same as looking up mesh incidences
    // as we use an exact kernel
    for (MeshEntityIterator entity(mesh,dim1); !entity.end(); ++entity)
    {
      // Compute intersection
      std::vector<std::size_t> ids_result;
      io.all_intersected_entities(*entity,ids_result);
      //sort them but they are already unique.
      std::sort(ids_result.begin(),ids_result.end());

      // Compute intersections via vertices and connectivity
      // information. Two entities of the same only intersect
      // if they share at least one verte
      std::vector<std::size_t> ids_result_2;
      if (dim1 > 0)
      {
        for (VertexIterator vertex(*entity); !vertex.end(); ++vertex)
        {
          std::size_t num_ent = vertex->num_entities(dim0);
          const unsigned int* entities = vertex->entities(dim0);
          for (std::size_t i = 0; i < num_ent; ++i)
            ids_result_2.push_back(entities[i]);
        }
      }
      // If we have a vertex simply take the incidences.
      else if (dim0 > 0)
      {
        std::size_t num_ent = entity->num_entities(dim0);
        const unsigned int* entities = entity->entities(dim0);
        for (std::size_t i = 0; i < num_ent; ++i)
          ids_result_2.push_back(entities[i]);
      }
      else
      {
        ids_result_2.push_back(entity->index());
      }
      //Sorting and removing duplicates
      std::sort(ids_result_2.begin(),ids_result_2.end());
      std::vector<std::size_t>::iterator it = std::unique(ids_result_2.begin(),ids_result_2.end());
      ids_result_2.resize(it - ids_result_2.begin());

      // Check against mesh incidences
      std::size_t last = ids_result.size() - 1;
      CPPUNIT_ASSERT(ids_result.size() == ids_result_2.size());
      CPPUNIT_ASSERT(ids_result[0] == ids_result_2[0]);
      CPPUNIT_ASSERT(ids_result[last] == ids_result_2[last]);
    }
  }

class IntersectionOperator3D : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(IntersectionOperator3D);

  CPPUNIT_TEST(testCellCellIntersection);
  CPPUNIT_TEST(testCellFacetIntersection);
  //Intersection betweenn tets and segments does not work yet
  //CPPUNIT_TEST(testCellEdgeIntersection);
  CPPUNIT_TEST(testCellVertexIntersection);

  CPPUNIT_TEST(testFacetFacetIntersection);
  CPPUNIT_TEST(testFacetEdgeIntersection);
  CPPUNIT_TEST(testFacetVertexIntersection);

  CPPUNIT_TEST(testEdgeEdgeIntersection);
  CPPUNIT_TEST(testEdgeVertexIntersection);
  CPPUNIT_TEST(testVertexVertexIntersection);

  CPPUNIT_TEST(testClosestPointQueries);

  CPPUNIT_TEST_SUITE_END();

public:

  void testCellCellIntersection()
  {
    std::size_t N = 3;
    UnitCubeMesh mesh(N, N, N);
    testEntityEntityIntersection<3, 3>(mesh);
  }

  void testCellFacetIntersection()
  {
    std::size_t N = 3;
    UnitCubeMesh mesh(N, N, N);
    testEntityEntityIntersection<3, 2>(mesh);
  }

  void testCellEdgeIntersection()
  {
    std::size_t N = 3;
    UnitCubeMesh mesh(N, N, N);
    testEntityEntityIntersection<3, 1>(mesh);
  }

  void testCellVertexIntersection()
  {
    std::size_t N = 3;
    UnitCubeMesh mesh(N, N, N);
    testEntityEntityIntersection<3, 0>(mesh);
  }

  void testFacetFacetIntersection()
  {
    std::size_t N = 3;
    UnitCubeMesh mesh(N, N, N);
    testEntityEntityIntersection<2, 2>(mesh);
  }

  void testFacetEdgeIntersection()
  {
    std::size_t N = 3;
    UnitCubeMesh mesh(N, N, N);
    testEntityEntityIntersection<2, 1>(mesh);
  }

  void testFacetVertexIntersection()
  {
    std::size_t N = 3;
    UnitCubeMesh mesh(N, N, N);
    testEntityEntityIntersection<2, 0>(mesh);
  }

  void testEdgeEdgeIntersection()
  {
    std::size_t N = 3;
    UnitCubeMesh mesh(N, N, N);
    testEntityEntityIntersection<1, 1>(mesh);
  }

  void testEdgeVertexIntersection()
  {
    std::size_t N = 3;
    UnitCubeMesh mesh(N, N, N);
    testEntityEntityIntersection<1, 0>(mesh);
  }

  void testVertexVertexIntersection()
  {
    std::size_t N = 3;
    UnitCubeMesh mesh(N, N, N);
    testEntityEntityIntersection<0, 0>(mesh);
  }

  void testClosestPointQueries()
  {
    UnitCubeMesh mesh(2, 2, 1);

    // Test distance queries for points outside mesh
    Point p(0.25,-0.5,0.1);
    CPPUNIT_ASSERT_EQUAL(mesh.closest_cell(p), size_t(1));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(0.75,-0.5,0.1);
    CPPUNIT_ASSERT_EQUAL(mesh.closest_cell(p), size_t(7));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(1.5,0.25,0.1);
    CPPUNIT_ASSERT_EQUAL(mesh.closest_cell(p), size_t(6));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(1.5,0.75,0.1);
    CPPUNIT_ASSERT_EQUAL(mesh.closest_cell(p), size_t(18));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(0.75,1.5,0.1);
    CPPUNIT_ASSERT_EQUAL(mesh.closest_cell(p), size_t(21));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(0.25,1.5,0.1);
    CPPUNIT_ASSERT_EQUAL(mesh.closest_cell(p), size_t(15));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(-0.5,0.75,0.1);
    CPPUNIT_ASSERT_EQUAL(mesh.closest_cell(p), size_t(17));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(-0.5,0.25,0.1);
    CPPUNIT_ASSERT_EQUAL(mesh.closest_cell(p), size_t(5));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    // Test distance queries for points inside mesh
    for (CellIterator cell(mesh); !cell.end(); ++cell)
    {
      Point p = cell->midpoint();
      CPPUNIT_ASSERT_EQUAL(mesh.closest_cell(p) ,size_t(cell->index()));
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, mesh.distance(p), DOLFIN_EPS);
    }
  }

};

class IntersectionOperator2D : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(IntersectionOperator2D);

  CPPUNIT_TEST(testCellCellIntersection);
  CPPUNIT_TEST(testCellEdgeIntersection);
  CPPUNIT_TEST(testCellVertexIntersection);

  CPPUNIT_TEST(testEdgeEdgeIntersection);
  CPPUNIT_TEST(testEdgeVertexIntersection);

  CPPUNIT_TEST(testVertexVertexIntersection);

  CPPUNIT_TEST(testClosestPointQueries);

  CPPUNIT_TEST_SUITE_END();

public:

  void testCellCellIntersection()
  {
    std::size_t N = 6;
    UnitSquareMesh mesh(N, N);
    testEntityEntityIntersection<2, 2>(mesh);
  }

  void testCellEdgeIntersection()
  {
    std::size_t N = 6;
    UnitSquareMesh mesh(N, N);
    testEntityEntityIntersection<2, 1>(mesh);
  }

  void testCellVertexIntersection()
  {
    std::size_t N = 6;
    UnitSquareMesh mesh(N, N);
    testEntityEntityIntersection<2, 0>(mesh);
  }

  void testEdgeEdgeIntersection()
  {
    std::size_t N = 6;
    UnitSquareMesh mesh(N, N);
    testEntityEntityIntersection<1, 1>(mesh);
  }

  void testEdgeVertexIntersection()
  {
    std::size_t N = 6;
    UnitSquareMesh mesh(N, N);
    testEntityEntityIntersection<1, 0>(mesh);
  }

  void testVertexVertexIntersection()
  {
    std::size_t N = 6;
    UnitSquareMesh mesh(N, N);
    testEntityEntityIntersection<0, 0>(mesh);
  }

  void testClosestPointQueries()
  {
    std::size_t N = 2;
    UnitSquareMesh mesh(N, N);

    // Test distance queries for points outside mesh
    Point p(0.25,-0.5,0.0);
    CPPUNIT_ASSERT_EQUAL(size_t(0), mesh.closest_cell(p));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(0.75,-0.5,0.0);
    CPPUNIT_ASSERT_EQUAL(size_t(2), mesh.closest_cell(p));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(1.5,0.25,0.0);
    CPPUNIT_ASSERT_EQUAL(size_t(2), mesh.closest_cell(p));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(1.5,0.75,0.0);
    CPPUNIT_ASSERT_EQUAL(size_t(6), mesh.closest_cell(p));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(0.75,1.5,0.0);
    CPPUNIT_ASSERT_EQUAL(size_t(7), mesh.closest_cell(p));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(0.25,1.5,0.0);
    CPPUNIT_ASSERT_EQUAL(size_t(5),mesh.closest_cell(p));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(-0.5,0.75,0.0);
    CPPUNIT_ASSERT_EQUAL(size_t(5), mesh.closest_cell(p));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    p = Point(-0.5,0.25,0.0);
    CPPUNIT_ASSERT_EQUAL(size_t(1), mesh.closest_cell(p));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.5, mesh.distance(p), DOLFIN_EPS);

    // Test distance queries for points inside mesh
    for (CellIterator cell(mesh); !cell.end(); ++cell)
    {
      Point p = cell->midpoint();
      CPPUNIT_ASSERT_EQUAL(size_t(cell->index()), mesh.closest_cell(p));
      CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, mesh.distance(p), DOLFIN_EPS);
    }
  }

};

class IntersectionOperator1D : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(IntersectionOperator1D);

  CPPUNIT_TEST(testCellCellIntersection);
  CPPUNIT_TEST(testCellVertexIntersection);

  CPPUNIT_TEST(testVertexVertexIntersection);

  CPPUNIT_TEST_SUITE_END();

public:

  void testCellCellIntersection()
  {
    std::size_t N = 10;
    UnitIntervalMesh mesh(N);
    testEntityEntityIntersection<1,1>(mesh);
  }

  void testCellVertexIntersection()
  {
    std::size_t N = 10;
    UnitIntervalMesh mesh(N);
    testEntityEntityIntersection<1,0>(mesh);
  }

  void testVertexVertexIntersection()
  {
    std::size_t N = 10;
    UnitIntervalMesh mesh(N);
    testEntityEntityIntersection<0,0>(mesh);
  }

};

int main()
{
  // FIXME: The following tests break probably in parallel
  if (dolfin::MPI::num_processes() == 1)
  {
    CPPUNIT_TEST_SUITE_REGISTRATION(IntersectionOperator3D);
    CPPUNIT_TEST_SUITE_REGISTRATION(IntersectionOperator2D);
    CPPUNIT_TEST_SUITE_REGISTRATION(IntersectionOperator1D);
  }

  DOLFIN_TEST;

}
