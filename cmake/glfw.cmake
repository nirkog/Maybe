FetchContent_Declare(
	Glfw
  	GIT_REPOSITORY 	git@github.com:glfw/glfw.git
  	GIT_TAG        	45ce5ddd197d5c58f50fdd3296a5131c894e5527
)

FetchContent_MakeAvailable(Glfw)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
