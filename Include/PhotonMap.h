#ifndef PHOTONMAP_H
#define PHOTONMAP_H

#include "nanoflann.hpp"
#include "ModelLoader.h"
#include "Raytracer.h"
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

namespace model {
	class Scene;
}

namespace photonmap {
	class PhotonMapper;
	template <typename T>
	struct PointCloud {
		struct Point {
			T  x, y, z;
			inline Point (glm::vec3 v) : x(v.x), y(v.y), z(v.z) {}
		};

		std::vector<Point>  pts;

		// Must return the number of data points
		inline size_t kdtree_get_point_count() const { return pts.size(); }

		// Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
		inline T kdtree_distance(const T *p1, const size_t idx_p2, size_t /*size*/) const {
			const T d0 = p1[0] - pts[idx_p2].x;
			const T d1 = p1[1] - pts[idx_p2].y;
			const T d2 = p1[2] - pts[idx_p2].z;
			return d0*d0 + d1*d1 + d2*d2;
		}

		// Returns the dim'th component of the idx'th point in the class:
		// Since this is inlined and the "dim" argument is typically an immediate value, the
		//  "if/else's" are actually solved at compile time.
		inline T kdtree_get_pt(const size_t idx, int dim) const {
			if (dim == 0) return pts[idx].x;
			else if (dim == 1) return pts[idx].y;
			else return pts[idx].z;
		}

		// Optional bounding-box computation: return false to default to a standard bbox computation loop.
		//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
		//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
		template <class BBOX>
		bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }

	};

	typedef nanoflann::KDTreeSingleIndexAdaptor<
		nanoflann::L2_Simple_Adaptor<float, PhotonMapper>,
		PhotonMapper, 3> photon_tree_t;

	class PhotonMap {
		photon_tree_t photon_map;
		PhotonMapper& p;
	public:
		PhotonMap(PhotonMapper* _p);
		glm::vec3 gatherPhotons(glm::vec3 point, glm::vec3 normal);
		//Finds the N nearest neighbours to a given point and returns the indices + the squared euclidean distance to each
		void findNNearestPoints(glm::vec3 point, std::vector<size_t>& ret_index, std::vector<float>& out_dist_sqr, unsigned int number_of_results = 5);
	};

	struct PhotonInfo {
		glm::vec3 color;
		glm::vec3 position;
		glm::vec3 direction;
		inline PhotonInfo(glm::vec3 c, glm::vec3 p, glm::vec3 d) : color(c), position(p), direction(d) {}
	};

	struct Photon {
		Ray beam;
		//Need to accumulate color
		unsigned int depth;
		bool absorbed = false;
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); //Photon is initially white
		Photon(glm::vec3 o, glm::vec3 d, unsigned int depth = 3);
	};

	class PhotonMapper {
		unsigned int number_of_photons;
		unsigned int number_of_bounces;

		std::vector<PhotonInfo> gathered_photons;
		std::vector<Photon> photons;

	public:
		inline PhotonMapper(model::Scene& scene, unsigned int photon_count = 1000, unsigned int bounces = 3) : number_of_photons(photon_count), number_of_bounces(bounces) {
			photons.reserve(number_of_photons);
			mapScene(scene);
		}

		void mapScene(model::Scene& scene); //Generate photons for each light source, distributed by light intensity

		inline glm::vec3 getDirection(size_t i) { return gathered_photons[i].direction; }
		inline glm::vec3 getEnergy(size_t i) { return gathered_photons[i].color; }



											// Must return the number of data points
		inline size_t kdtree_get_point_count() const { return gathered_photons.size(); }

		// Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
		inline float kdtree_distance(const float* p1, const size_t idx_p2, size_t /*size*/) const {
			const float d0 = p1[0] - gathered_photons[idx_p2].position.x;
			const float d1 = p1[1] - gathered_photons[idx_p2].position.y;
			const float d2 = p1[2] - gathered_photons[idx_p2].position.z;
			return d0*d0 + d1*d1 + d2*d2;
		}

		// Returns the dim'th component of the idx'th point in the class:
		// Since this is inlined and the "dim" argument is typically an immediate value, the
		//  "if/else's" are actually solved at compile time.
		inline float kdtree_get_pt(const size_t idx, int dim) const {
			if (dim == 0) return gathered_photons[idx].position.x;
			else if (dim == 1) return gathered_photons[idx].position.y;
			else return gathered_photons[idx].position.z;
		}

		// Optional bounding-box computation: return false to default to a standard bbox computation loop.
		//   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
		//   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
		template <class BBOX>
		bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }
	};
}

#endif // PHOTONMAP_H

