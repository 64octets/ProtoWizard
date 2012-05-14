#pragma once;
#include <string>
#include <memory>

namespace protowizard{
	class Mesh;
	typedef std::shared_ptr<Mesh> MeshPtr;

	class MeshManager
	{
	public:
		virtual ~MeshManager() {}
		virtual MeshPtr getMesh( const std::string& file_path ) = 0;

		static MeshManager *init();
		static void shutdown(MeshManager *tm);
	};
}