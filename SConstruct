#libs = ['SDLmain', 'SDL_ttf', 'SDL', 'glut', 'GLU', 'GL', 'jpeg', 'boost_filesystem-mt', 'boost_system-mt', 'boost_iostreams-mt', 'ftgl', 'freetype', 'ode']
libs = ['jpeg', 'boost_filesystem-xgcc40-mt', 'boost_system-xgcc40-mt', 'boost_iostreams-xgcc40-mt', 'SDL_ttf', 'SDL_image', 'SDLmain', 'SDL']

env = Environment(LIBPATH='/opt/local/lib/', CPPFLAGS='-g -I/opt/local/include/ -I/opt/local/include/boost-1_39/ -Wall', FRAMEWORKS=['Cocoa', 'OpenGL'])
conf = Configure(env)

for lib in libs:
	if not conf.CheckLib(lib):
		print lib + " not found"
		Exit(1)

# Get SDL
#env.ParseConfig('/opt/local/bin/sdl-config --cflags')
#env.ParseConfig('/opt/local/bin/sdl-config --libs')

# and ode
env.ParseConfig('/usr/local/bin/ode-config --cflags')
env.ParseConfig('/usr/local/bin/ode-config --libs')
env.Append(LIBS = ['ode'])


env = conf.Finish()

env.Program('raceya', Glob('src/*.cpp'))
