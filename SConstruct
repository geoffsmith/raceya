#libs = ['SDLmain', 'SDL_ttf', 'SDL', 'glut', 'GLU', 'GL', 'jpeg', 'boost_filesystem-mt', 'boost_system-mt', 'boost_iostreams-mt', 'ftgl', 'freetype', 'ode']
libs = ['jpeg', 'boost_filesystem-mt', 'boost_iostreams-mt', 'boost_system-mt', 'GL', 'GLU', 'SDL_image', 'SDL_ttf']

env = Environment(LIBPATH='/usr/lib/', CPPFLAGS='-D GL_GLEXT_PROTOTYPES -g -I/usr/include/ -Wall')
#define GL_GLEXT_PROTOTYPES
conf = Configure(env)

for lib in libs:
	if not conf.CheckLib(lib):
		print lib + " not found"
		Exit(1)

# Get SDL
env.ParseConfig('sdl-config --cflags')
env.ParseConfig('sdl-config --libs')

# and ode
env.ParseConfig('ode-config --cflags')
env.ParseConfig('ode-config --libs')
env.Append(LIBS = ['ode'])


env = conf.Finish()

env.Program('raceya', Glob('src/*.cpp'))
