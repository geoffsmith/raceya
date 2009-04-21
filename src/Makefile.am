bin_PROGRAMS = raceya
raceya_SOURCES = camera.cpp car.cpp closest_point.cpp dof.cpp frame_timer.cpp frustum_culler.cpp hud.cpp jpeg.cpp lib.cpp logger.cpp main.cpp matrix.cpp shader.cpp track.cpp wheel.cpp camera.h car.h closest_point.h dof.h frame_timer.h frustum_culler.h hud.h jpeg.h lib.h logger.h main.h matrix.h shader.h track.h wheel.h car_parser.h car_parser.cpp texture.h texture.cpp ini.h ini.cpp opengl_state.h opengl_state.cpp quaternion.h quaternion.cpp vector.h vector.cpp curve.h curve.cpp drive_systems.h drive_systems.cpp rigid_body.h rigid_body.cpp
raceya_LDADD = -lglut -lGLU -lGL -lSDL -lSDLmain -lSDL_ttf -lSDL_mixer -lSDL_image -ljpeg -lboost_filesystem-mt -lboost_system-mt -lboost_iostreams-mt -lftgl -lfreetype 
raceya_LDFLAGS = -framework Cocoa $(SDL_LIBS) $(GL_LIBS)
raceya_CXXFLAGS = $(SDL_CFLAGS) $(BOOST_CPPFLAGS) $(GL_CFLAGS) -Wall
