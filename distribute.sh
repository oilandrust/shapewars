echo Distibuting GetTheFlag
cd build/
xcodebuild -scheme GetTheFlag -configuration Release build
cd ../
if [ -d "dist" ]; then
	rm -r dist/
fi
mkdir dist/
mkdir dist/GetTheFlag/
cp build/GetTheFlag/Debug/GetTheFlag dist/GetTheFlag/
cp -r GetTheFlag/data dist/GetTheFlag/
cp -r GetTheFlag/shaders dist/GetTheFlag/
cp -r /Library/Frameworks/SDL2.framework dist/GetTheFlag/SDL2.framework
cp -r /Library/Frameworks/SDL2_image.framework dist/GetTheFlag/SDL2_image.framework
cd dist/
zip -r GetTheFlag.zip GetTheFlag/
cd ../
