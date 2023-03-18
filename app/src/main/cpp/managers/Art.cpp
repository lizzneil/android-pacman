/*
 * Art.cpp
 *
 *  Created on: 10.11.2012
 *      Author: Denis Zagayevskiy
 */

#include <cstring>
#include "Art.h"

const char* Art::PATH_LEVELS = "levels";
const char* Art::PATH_TEXTURES_SMALL = "textures/small/%s";
const char* Art::PATH_TEXTURES_MEDIUM = "textures/medium/%s";
const char* Art::PATH_TEXTURES_LARGE = "textures/large/%s";
const char* Art::PATH_GAME_BACKGROUND_MUSIC = "audio/music/game.ogg";
const char* Art::PATH_MENU_BACKGROUND_MUSIC = "audio/music/menu.ogg";

AAssetManager* Art::assetManager;
jobject Art::pngManager = NULL;
JNIEnv* Art::pmEnv;
jclass Art::pmClass;
jmethodID Art::pmOpenId;
jmethodID Art::pmCloseId;
jmethodID Art::pmGetWidthId;
jmethodID Art::pmGetHeightId;
jmethodID Art::pmGetPixelsId;

GLfloat Art::screenWidth = 0, Art::screenHeight = 0;

GLfloat* Art::MVPMatrix = NULL;

const char* Art::texturesPath = Art::PATH_TEXTURES_SMALL; //Default textures are small

Texture** Art::texturesSources = NULL;
GLuint* Art::textures = NULL;

char** Art::shadersSources = NULL;
GLuint* Art::shaderPrograms = NULL;

Level** Art::levels = NULL;
GLfloat** Art::levelsTexCoords = NULL;
int Art::levelsCount = 0;

SoundBuffer** Art::sounds = NULL;

ResourseDescriptor Art::gameMusicDescriptor = EMPTY_RESOURSE_DESCRIPTOR;
ResourseDescriptor Art::menuMusicDescriptor = EMPTY_RESOURSE_DESCRIPTOR;

void Art::init(JNIEnv* env, jint _screenWidth, jint _screenHeight, jobject _pngManager, jobject javaAssetManager){
	LOGI("Art::init");
	free(env);

	pngManager = env->NewGlobalRef(_pngManager);
	pmEnv = env;

	pmClass = env->GetObjectClass(pngManager);
	pmOpenId = env->GetMethodID(pmClass, "open", "(Ljava/lang/String;)Landroid/graphics/Bitmap;");
	pmCloseId = env->GetMethodID(pmClass, "close", "(Landroid/graphics/Bitmap;)V");
	pmGetWidthId = env->GetMethodID(pmClass, "getWidth", "(Landroid/graphics/Bitmap;)I");
	pmGetHeightId = env->GetMethodID(pmClass, "getHeight", "(Landroid/graphics/Bitmap;)I");
	pmGetPixelsId = env->GetMethodID(pmClass, "getPixels", "(Landroid/graphics/Bitmap;[I)V");

	assetManager = AAssetManager_fromJava(env, javaAssetManager);

	screenWidth = _screenWidth;
	screenHeight = _screenHeight;

	if(_screenWidth <= 480){
		texturesPath = PATH_TEXTURES_SMALL;
	}else if(_screenWidth <= 600){
		texturesPath = PATH_TEXTURES_MEDIUM;
	}else{
		texturesPath = PATH_TEXTURES_LARGE;
	}

	loadLevels();
	loadTextures();
	loadMusic();

	shadersSources = new char*[SHADERS_COUNT];
	shadersSources[SHADER_VERTEX_0] = loadTextFile("shaders/shader.vrt");
	shadersSources[SHADER_FRAGMENT_0] = loadTextFile("shaders/shader.frg");
	shadersSources[SHADER_VERTEX_SHIFT] = loadTextFile("shaders/shiftShader.vrt");
	shadersSources[SHADER_VERTEX_BRUSHES] = loadTextFile("shaders/brushes.vrt");
	shadersSources[SHADER_FRAGMENT_BRUSHES] = loadTextFile("shaders/brushes.frg");
	shadersSources[SHADER_VERTEX_MASK_OVERLAY] = loadTextFile("shaders/maskOverlay.vrt");
	shadersSources[SHADER_FRAGMENT_MASK_OVERLAY] = loadTextFile("shaders/maskOverlay.frg");

	MVPMatrix = generateMVPMatrix(_screenWidth, _screenHeight);
}

void Art::initOpenGL(){
	compilePrograms();
	generateTextures();
}

GLfloat* Art::getMVPMatrix(){
	return MVPMatrix;
}

GLuint Art::getTexture(int id){
	return (0 <= id && id < TEXTURES_COUNT && textures) ? textures[id] : TEXTURE_NONE;
}

char* Art::getShaderSource(int id){
	return (0 <= id && id < SHADERS_COUNT) ? shadersSources[id] : NULL;
}

GLuint Art::getShaderProgram(int id){
	return (0 <= id && id < SHADER_PROGRAMS_COUNT) ? shaderPrograms[id] : SHADER_PROGRAM_NONE;
}

Level* Art::getLevel(int number){
	return (number >= 0 && number < levelsCount) ? levels[number] : NULL;
}

GLfloat* Art::getLevelTexCoords(int number){
	return (number >= 0 && number < levelsCount) ? levelsTexCoords[number] : NULL;
}

ResourseDescriptor Art::getGameBackgroundMusicDescriptor(){
	return gameMusicDescriptor;
}

ResourseDescriptor Art::getMenuBackgroundMusicDescriptor(){
	return menuMusicDescriptor;
}

const SoundBuffer* Art::getSound(unsigned int id){
	return (id < SOUNDS_COUNT) ? sounds[id] : &EMPTY_SOUND_BUFFER;
}

void Art::free(JNIEnv* env){
	LOGI("Art::free");

	if(pngManager){
		env->DeleteGlobalRef(pngManager);
		pngManager = NULL;
	}

	if(MVPMatrix){
		delete[] MVPMatrix;
		MVPMatrix = NULL;
	}

	if(textures){
		glDeleteTextures(TEXTURES_COUNT, textures);
		delete[] textures;
		textures = NULL;
	}

	if(texturesSources){
		for(int i = 0; i < TEXTURES_COUNT; ++i){
			if(texturesSources[i]){
				delete texturesSources[i];
			}
		}
		delete[] texturesSources;
		texturesSources = NULL;
	}

	if(shadersSources){
		for(int i = 0; i < SHADERS_COUNT; ++i){
			if(shadersSources[i]){
				delete[] shadersSources[i];
			}
		}
		delete[] shadersSources;
		shadersSources = NULL;
	}

	if(shaderPrograms){
		for(int i = 0; i < SHADER_PROGRAMS_COUNT; ++i){
			glDeleteProgram(shaderPrograms[i]);
		}
		delete[] shaderPrograms;
		shaderPrograms = NULL;
	}

	if(levels){
		for(int i = 0; i < levelsCount; ++i){
			if(levels[i]){
				delete levels[i];
			}
		}
		delete[] levels;
		levels = NULL;
	}

	if(levelsTexCoords){
		for(int i = 0; i < levelsCount; ++i){
			if(levelsTexCoords[i]){
				delete levelsTexCoords[i];
			}
		}
		delete[] levelsTexCoords;
		levelsTexCoords = NULL;
	}

	if(sounds){
		for(int i = 0; i < SOUNDS_COUNT; ++i){
			if(sounds[i]){
				delete sounds[i];
				sounds[i] = NULL;
			}
		}
		delete[] sounds;
		sounds = NULL;
	}

}

Texture* Art::loadPng(const char* filename){
	LOGI("Art::loadPng(%s)", filename);

	Texture* result = new Texture();

	jstring name = pmEnv->NewStringUTF(filename);
	jobject png = pmEnv->CallObjectMethod(pngManager, pmOpenId, name);
	pmEnv->DeleteLocalRef(name);
	pmEnv->NewGlobalRef(png);

	jint width = pmEnv->CallIntMethod(pngManager, pmGetWidthId, png);
	jint height = pmEnv->CallIntMethod(pngManager, pmGetHeightId, png);

	jintArray array = pmEnv->NewIntArray(width * height);
	pmEnv->NewGlobalRef(array);
	pmEnv->CallVoidMethod(pngManager, pmGetPixelsId, png, array);

	jint *pixels = pmEnv->GetIntArrayElements(array, 0);

	result->pixels = argb2rgba((unsigned int*)pixels, width, height);
	result->width = width;
	result->height = height;

	pmEnv->ReleaseIntArrayElements(array, pixels, 0);
	pmEnv->CallVoidMethod(pngManager, pmCloseId, png);

	return result;
}

void Art::compilePrograms(){
	shaderPrograms = new GLuint[SHADER_PROGRAMS_COUNT];
	shaderPrograms[SHADER_PROGRAM_0] = ShadersManager::createProgram(shadersSources[SHADER_VERTEX_0], shadersSources[SHADER_FRAGMENT_0]);
	shaderPrograms[SHADER_PROGRAM_SHIFT] = ShadersManager::createProgram(
			shadersSources[SHADER_VERTEX_SHIFT],
			shadersSources[SHADER_FRAGMENT_0]
	);
	shaderPrograms[SHADER_PROGRAM_MASK_OVERLAY] = ShadersManager::createProgram(
			shadersSources[SHADER_VERTEX_MASK_OVERLAY],
			shadersSources[SHADER_FRAGMENT_MASK_OVERLAY]
	);
}

void Art::generateTextures(){
	LOGI("Art::generateTextures");

	textures = new GLuint[TEXTURES_COUNT];
	for(int i = 0; i < TEXTURES_COUNT; ++i){
		textures[i] = texturesSources[i] ? createTexture(texturesSources[i]) : TEXTURE_NONE;
	}
	textures[TEXTURE_BRUSHES] = generateBrushesTexture();
}

GLuint Art::createTexture(Texture* texture){
	GLuint textureId;
	glGenTextures(1, &textureId);

	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);

	return textureId;
}

GLuint Art::generateBrushesTexture(){
	GLuint textureId;
	GLuint program = ShadersManager::createProgram(Art::getShaderSource(Art::SHADER_VERTEX_BRUSHES), Art::getShaderSource(Art::SHADER_FRAGMENT_BRUSHES));
			//ShadersManager::createProgram(Art::getShaderSource(Art::SHADER_VERTEX_BRUSHES), Art::getShaderSource(Art::SHADER_FRAGMENT_BRUSHES));
	if(!program){
		return 0;
	}

	glUseProgram(program);

	GLuint positionHandle = glGetAttribLocation(program, "aPosition");
	checkGlError("getAttribLocation0");

	glViewport(0, 0, 1024, 1024);

	GLint oldFrameBufferId;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFrameBufferId);

	GLuint frameBufferId;
	glGenFramebuffers(1, &frameBufferId);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	GLfloat quad[] = {
		-1.0, -1.0, 1.0, -1.0, 1.0, 1.0,
		1.0, 1.0, -1.0, 1.0, -1.0, -1.0
	};

	glVertexAttribPointer(positionHandle, 2, GL_FLOAT, GL_FALSE, 0, quad);
	checkGlError("glVertexAttribPointer1");
	glEnableVertexAttribArray(positionHandle);
	checkGlError("glEnableVertexAttribArray2");

	glDrawArrays(GL_TRIANGLES, 0, 6);
	checkGlError("glDrawArrays3");

	glDisableVertexAttribArray(positionHandle);

	glBindFramebuffer(GL_FRAMEBUFFER, oldFrameBufferId);
	glDeleteFramebuffers(1, &frameBufferId);

	glDeleteProgram(program);

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);\

	glViewport(0, 0, screenWidth, screenHeight);

	return textureId;
}

GLfloat* Art::generateMVPMatrix(int w, int h){
	float near = 1.0, far = -1.0;
	float left = 0.0, right = 1.0f / (float) h * (float) w, bottom = 1.0, top = 0.0;
	GLfloat* matrix = new GLfloat[16];

	// First Column
	matrix[0] = 2.0 / (right - left);
	matrix[1] = 0.0;
	matrix[2] = 0.0;
	matrix[3] = 0.0;

	// Second Column
	matrix[4] = 0.0;
	matrix[5] = 2.0 / (top - bottom);
	matrix[6] = 0.0;
	matrix[7] = 0.0;

	// Third Column
	matrix[8] = 0.0;
	matrix[9] = 0.0;
	matrix[10] = -2.0 / (far - near);
	matrix[11] = 0.0;

	// Fourth Column
	matrix[12] = -(right + left) / (right - left);
	matrix[13] = -(top + bottom) / (top - bottom);
	matrix[14] = -(far + near) / (far - near);
	matrix[15] = 1;

	return matrix;
}

char* Art::loadTextFile(const char* filename){
	LOGI("Art::loadTextFile(%s)", filename);

	AAsset* asset = AAssetManager_open(assetManager, filename, AASSET_MODE_BUFFER);
	long length = AAsset_getLength(asset);

	char* buffer = new char[length];
	long offset = 0;
	long readed = 0;
	while((readed = AAsset_read(asset, buffer + offset, length - offset)) > 0){
		offset += readed;
	}

	AAsset_close(asset);

	buffer[length - 1] = '\0';

	return buffer;
}

char* Art::argb2rgba(unsigned int* pixels, int w, int h){
	char* result = new char[w*h*4];

	unsigned int pixel;
	int i, j, offset;

	for(i = 0; i < h; ++i){
		offset = i*w*4;
		for(j = 0; j < w*4; j += 4){
			pixel = pixels[i*w + j / 4];
			result[offset + j] 		= (pixel >> 16) & 0xff;
			result[offset + j + 1]  = (pixel >> 8) & 0xff;
			result[offset + j + 2]  = pixel & 0xff;
			result[offset + j + 3]  = (pixel >> 24) & 0xff;
		}
	}
	return result;
}

List<char*> Art::loadFilesList(const char* path){
	LOGI("Art::loadFilesList(%s)", path);

	List<char*> result;
	AAssetDir* assetDir = AAssetManager_openDir(assetManager, path);
	const char* c;
	while(((c = AAssetDir_getNextFileName(assetDir)) != NULL)){
		char * buffer = new char[MAX_PATH];
		sprintf(buffer, "%s", c);
		result.pushTail(buffer);
	}

	AAssetDir_close(assetDir);

	return result;
}

List<ResourseDescriptor> Art::loadFilesDescriptorsList(const char* path){
	List<ResourseDescriptor> result;
	char buffer[MAX_PATH];
	List<char*> files = loadFilesList(path);

	LOGI("loadFilesDescriptors from %s, count: %i", path, files.getLength());

	char* fileName = NULL;
	bool exists = files.getHead(fileName);
	while(exists){
		sprintf(buffer, "%s/%s", path, fileName);
		AAsset* asset = AAssetManager_open(assetManager, buffer, AASSET_MODE_UNKNOWN);
		ResourseDescriptor resourceDescriptor;
		resourceDescriptor.decriptor = AAsset_openFileDescriptor(asset, &resourceDescriptor.start, &resourceDescriptor.length);
		result.pushTail(resourceDescriptor);
		AAsset_close(asset);

		LOGI("File descriptor loaded: %s", buffer);

		exists = files.getNext(fileName);
	}
	files.clear();
	return result;
}

ResourseDescriptor Art::loadResourceDescriptor(const char* path){
	AAsset* asset = AAssetManager_open(assetManager, path, AASSET_MODE_UNKNOWN);
	ResourseDescriptor resourceDescriptor;
	resourceDescriptor.decriptor = AAsset_openFileDescriptor(asset, &resourceDescriptor.start, &resourceDescriptor.length);
	AAsset_close(asset);
	return resourceDescriptor;
}

void Art::loadLevels(){
	LOGI("Art::loadLevels");
	List<char*> files = loadFilesList(PATH_LEVELS);
	levelsCount = files.getLength();
	char buffer[MAX_PATH];
	if(!files.isEmpty()){
		levels = new Level*[levelsCount];
		char* file;
		bool exists = files.getHead(file);
		int i = 0;
		while(exists){
			sprintf(buffer, "%s/%s", PATH_LEVELS, file);
			levels[i++] = new Level(file, loadPng(buffer));
			exists = files.getNext(file);
		}
		files.clear();
	}
}

void Art::loadTextures(){
	LOGI("Art:loadTextures");
	char buffer[MAX_PATH];
	texturesSources = new Texture*[TEXTURES_COUNT];
	sprintf(buffer, texturesPath, "tiles.png");
	LOGI("%s", buffer);
	texturesSources[TEXTURE_TILES] = loadPng(buffer);
	sprintf(buffer, texturesPath, "buttons.png");
	texturesSources[TEXTURE_BUTTONS] = loadPng(buffer);
	sprintf(buffer, texturesPath, "pacmans.png");
	texturesSources[TEXTURE_PACMAN_ANIMATION] = loadPng(buffer);
	texturesSources[TEXTURE_FONT_CONSOLAS] = loadPng("textures/font_consolas.png");
	texturesSources[TEXTURE_ALL_LEVELS] = makeTextureFromLevels();
	texturesSources[TEXTURE_BRUSHES] = NULL;
	texturesSources[TEXTURE_FIRE] = loadPng("textures/fire.png");
	texturesSources[TEXTURE_WATER] = loadPng("textures/water.png");
	texturesSources[TEXTURE_GREEN_GOO] = loadPng("textures/green_goo.png");
}

void Art::loadMusic(){
	LOGI("Art::loadMusic");

	sounds = new SoundBuffer*[SOUNDS_COUNT];
	sounds[SOUND_LIFE] = loadSoundFile("audio/sounds/life.wav");
	sounds[SOUND_DEATH] = loadSoundFile("audio/sounds/death.wav");
	sounds[SOUND_WIN] = loadSoundFile("audio/sounds/win.wav");
	sounds[SOUND_GAMEOVER] = loadSoundFile("audio/sounds/gameOver.wav");
	sounds[SOUND_CLICK] = loadSoundFile("audio/sounds/click.wav");

	gameMusicDescriptor = loadResourceDescriptor(PATH_GAME_BACKGROUND_MUSIC);
	menuMusicDescriptor = loadResourceDescriptor(PATH_MENU_BACKGROUND_MUSIC);
}

SoundBuffer* Art::loadSoundFile(const char* filename){
	SoundBuffer* result = new SoundBuffer();
	AAsset* asset = AAssetManager_open(assetManager, filename, AASSET_MODE_UNKNOWN);
	off_t length = AAsset_getLength(asset);
	result->length = length - sizeof(WAVHeader);
	result->header = new WAVHeader();
	result->buffer = new char[result->length + 1];
	AAsset_read(asset, result->header, sizeof(WAVHeader));
	AAsset_read(asset, result->buffer, result->length);
	result->buffer[result->length] = '\0';
	AAsset_close(asset);
	return result;
}

/*
 * 		Method return big image with all (<= MAX_LEVELS_COUNT) levels printed on it
 * and fill levelsTexCoords array with texture coords of levels.
 */
Texture* Art::makeTextureFromLevels(){
	LOGI("Art::makeTextureFromLevels");

	if(levelsCount <= 0){
		return NULL;
	}

	int w = LEVELS_ON_SIDE_COUNT*MAX_LEVEL_SIZE;
	int h = w;
	char* pixels = new char[w*h*4];
	std::memset(pixels, 0, w*h*4*sizeof(char));

	levelsTexCoords = new GLfloat*[levelsCount];
	float fWidth = (float) LEVELS_ON_SIDE_COUNT*MAX_LEVEL_SIZE;
	float fHeight = fWidth;

	for(int k = 0; k < levelsCount && k < MAX_LEVELS_COUNT; ++k){

		/*Compute position of this level*/
		int posX = k % LEVELS_ON_SIDE_COUNT;
		int posY = k / LEVELS_ON_SIDE_COUNT;

		int globalOffset =
				posX*MAX_LEVEL_SIZE*4 +
				posY*MAX_LEVEL_SIZE*MAX_LEVEL_SIZE*LEVELS_ON_SIDE_COUNT*4;

		Texture* currentLevel = levels[k]->map;
		int iLevelWidth = currentLevel->width < MAX_LEVEL_SIZE ? currentLevel->width : MAX_LEVEL_SIZE;
		int iLevelHeight = currentLevel->height < MAX_LEVEL_SIZE ? currentLevel->height : MAX_LEVEL_SIZE;


		/*Print level image to big image*/
		for(int i = 0; i < iLevelHeight; ++i){
			memcpy(&(pixels[globalOffset + i*MAX_LEVEL_SIZE*LEVELS_ON_SIDE_COUNT*4]), &(currentLevel->pixels[i*currentLevel->width*4]), iLevelWidth*4);
		}

		/*Compute texture coords of this level in big texture*/
		float fLevelWidth = float(iLevelWidth) / (float)(MAX_LEVEL_SIZE*LEVELS_ON_SIDE_COUNT);
		float fLevelHeight = float(iLevelHeight) / (float)(MAX_LEVEL_SIZE*LEVELS_ON_SIDE_COUNT);
		float fX = (float)posX / float(LEVELS_ON_SIDE_COUNT);
		float fY = (float)posY / float(LEVELS_ON_SIDE_COUNT);

		GLfloat tempCoords[12] = {
			fX, fY, fX + fLevelWidth, fY, fX + fLevelWidth, fY + fLevelHeight,
			fX + fLevelWidth, fY + fLevelHeight, fX, fY + fLevelHeight, fX, fY
		};

		levelsTexCoords[k] = new GLfloat[12];

		for(int i = 0; i < 12; ++i){
			levelsTexCoords[k][i] = tempCoords[i];
		}

	}

	return new Texture(pixels, w, h);

}

#define BUTTON_SIZE 0.25f

GLfloat Art::TEX_COORDS_BUTTON_LEFT[12] = {
	BUTTON_SIZE, BUTTON_SIZE, 0.0, BUTTON_SIZE, 0.0, 0.0,
	0.0, 0.0, BUTTON_SIZE, 0.0, BUTTON_SIZE, BUTTON_SIZE
};

GLfloat Art::TEX_COORDS_BUTTON_LEFT_PRESSED[12] = {
	2*BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, 0.0,
	BUTTON_SIZE, 0.0, 2*BUTTON_SIZE, 0.0, 2*BUTTON_SIZE, BUTTON_SIZE
};

GLfloat Art::TEX_COORDS_BUTTON_RIGHT[12] = {
	0.0, 0.0, BUTTON_SIZE, 0.0, BUTTON_SIZE, BUTTON_SIZE,
	BUTTON_SIZE, BUTTON_SIZE, 0.0, BUTTON_SIZE, 0.0, 0.0
};

GLfloat Art::TEX_COORDS_BUTTON_RIGHT_PRESSED[12] = {
	BUTTON_SIZE, 0.0, 2*BUTTON_SIZE, 0.0, 2*BUTTON_SIZE, BUTTON_SIZE,
	2*BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, 0.0
};

GLfloat Art::TEX_COORDS_BUTTON_UP[12] = {
	BUTTON_SIZE, 0.0, BUTTON_SIZE, BUTTON_SIZE, 0.0, BUTTON_SIZE,
	0.0, BUTTON_SIZE, 0.0, 0.0, BUTTON_SIZE, 0.0
};

GLfloat Art::TEX_COORDS_BUTTON_UP_PRESSED[12] = {
	2*BUTTON_SIZE, 0.0, 2*BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE,
	BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, 0.0, 2*BUTTON_SIZE, 0.0
};

GLfloat Art::TEX_COORDS_BUTTON_DOWN[12] = {
	0.0, BUTTON_SIZE, 0.0, 0.0, BUTTON_SIZE, 0.0,
	BUTTON_SIZE, 0.0, BUTTON_SIZE, BUTTON_SIZE, 0.0, BUTTON_SIZE
};

GLfloat Art::TEX_COORDS_BUTTON_DOWN_PRESSED[12] = {
	BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, 0.0, 2*BUTTON_SIZE, 0.0,
	2*BUTTON_SIZE, 0.0, 2*BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE
};


GLfloat Art::TEX_COORDS_BUTTON_PLAY[12] = {
	0.0, 0.0, BUTTON_SIZE, 0.0, BUTTON_SIZE, BUTTON_SIZE,
	BUTTON_SIZE, BUTTON_SIZE, 0.0, BUTTON_SIZE, 0.0, 0.0
};

GLfloat Art::TEX_COORDS_BUTTON_PLAY_PRESSED[12] = {
	BUTTON_SIZE, 0.0, 2*BUTTON_SIZE, 0.0, 2*BUTTON_SIZE, BUTTON_SIZE,
	2*BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, 0.0
};

GLfloat Art::TEX_COORDS_BUTTON_RETRY[12] = {
	2*BUTTON_SIZE, 0.0, 3*BUTTON_SIZE, 0.0, 3*BUTTON_SIZE, BUTTON_SIZE,
	3*BUTTON_SIZE, BUTTON_SIZE, 2*BUTTON_SIZE, BUTTON_SIZE, 2*BUTTON_SIZE, 0.0
};

GLfloat Art::TEX_COORDS_BUTTON_RETRY_PRESSED[12] = {
	3*BUTTON_SIZE, 0.0, 4*BUTTON_SIZE, 0.0, 4*BUTTON_SIZE, BUTTON_SIZE,
	4*BUTTON_SIZE, BUTTON_SIZE, 3*BUTTON_SIZE, BUTTON_SIZE, 3*BUTTON_SIZE, 0.0
};

GLfloat Art::TEX_COORDS_BUTTON_NEXT[12] = {
	0.0, BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE, 2*BUTTON_SIZE,
	BUTTON_SIZE, 2*BUTTON_SIZE, 0.0, 2*BUTTON_SIZE, 0.0, BUTTON_SIZE
};

GLfloat Art::TEX_COORDS_BUTTON_NEXT_PRESSED[12] = {
	BUTTON_SIZE, BUTTON_SIZE, 2*BUTTON_SIZE, BUTTON_SIZE, 2*BUTTON_SIZE, 2*BUTTON_SIZE,
	2*BUTTON_SIZE, 2*BUTTON_SIZE, BUTTON_SIZE, 2*BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE
};

GLfloat Art::TEX_COORDS_BUTTON_LIST[12] = {
	2*BUTTON_SIZE, BUTTON_SIZE, 3*BUTTON_SIZE, BUTTON_SIZE, 3*BUTTON_SIZE, 2*BUTTON_SIZE,
	3*BUTTON_SIZE, 2*BUTTON_SIZE, 2*BUTTON_SIZE, 2*BUTTON_SIZE, 2*BUTTON_SIZE, BUTTON_SIZE
};

GLfloat Art::TEX_COORDS_BUTTON_LIST_PRESSED[12] = {
	3*BUTTON_SIZE, BUTTON_SIZE, 4*BUTTON_SIZE, BUTTON_SIZE, 4*BUTTON_SIZE, 2*BUTTON_SIZE,
	4*BUTTON_SIZE, 2*BUTTON_SIZE, 3*BUTTON_SIZE, 2*BUTTON_SIZE, 3*BUTTON_SIZE, BUTTON_SIZE
};

GLfloat Art::TEX_COORDS_BUTTON_MUSIC_ON[12] = {
	0.0, 2*BUTTON_SIZE, BUTTON_SIZE, 2*BUTTON_SIZE, BUTTON_SIZE, 3*BUTTON_SIZE,
	BUTTON_SIZE, 3*BUTTON_SIZE, 0.0, 3*BUTTON_SIZE, 0.0, 2*BUTTON_SIZE
};

GLfloat Art::TEX_COORDS_BUTTON_MUSIC_OFF[12] = {
	BUTTON_SIZE, 2*BUTTON_SIZE, 2*BUTTON_SIZE, 2*BUTTON_SIZE, 2*BUTTON_SIZE, 3*BUTTON_SIZE,
	2*BUTTON_SIZE, 3*BUTTON_SIZE, BUTTON_SIZE, 3*BUTTON_SIZE, BUTTON_SIZE, 2*BUTTON_SIZE
};

GLfloat Art::TEX_COORDS_BUTTON_SOUNDS_ON[12] = {
	2*BUTTON_SIZE, 2*BUTTON_SIZE, 3*BUTTON_SIZE, 2*BUTTON_SIZE, 3*BUTTON_SIZE, 3*BUTTON_SIZE,
	3*BUTTON_SIZE, 3*BUTTON_SIZE, 2*BUTTON_SIZE, 3*BUTTON_SIZE, 2*BUTTON_SIZE, 2*BUTTON_SIZE
};

GLfloat Art::TEX_COORDS_BUTTON_SOUNDS_OFF[12] = {
	3*BUTTON_SIZE, 2*BUTTON_SIZE, 4*BUTTON_SIZE, 2*BUTTON_SIZE, 4*BUTTON_SIZE, 3*BUTTON_SIZE,
	4*BUTTON_SIZE, 3*BUTTON_SIZE, 3*BUTTON_SIZE, 3*BUTTON_SIZE, 3*BUTTON_SIZE, 2*BUTTON_SIZE
};

#define TILE_SIZE 0.5f

GLfloat Art::TEX_COORDS_TILE_FREE[8] = {
	0.0, 0.0,
	TILE_SIZE, 0.0,
	TILE_SIZE, TILE_SIZE,
	0.0, TILE_SIZE
};

GLfloat Art::TEX_COORDS_TILE_WALL[8] = {
	TILE_SIZE, 0.0,
	2*TILE_SIZE, 0.0,
	2*TILE_SIZE, TILE_SIZE,
	TILE_SIZE, TILE_SIZE
};

GLfloat Art::TEX_COORDS_TILE_FOOD[8] = {
	0.0, TILE_SIZE,
	TILE_SIZE, TILE_SIZE,
	TILE_SIZE, 2*TILE_SIZE,
	0.0, 2*TILE_SIZE
};
