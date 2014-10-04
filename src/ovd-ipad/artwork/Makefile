ICON=icon.svgz
LOGO=logo.svgz

APP_IMAGES=FloatingToolbarBG.png FloatingToolbarHelp.png FloatingToolbarQuit.png toolbar.png ulteo-bandeau-light.jpg
APP_IMAGES_BRANDING=32.png 128.png ulteo.big.png ulteo_big_logo_text.png logo_small.png
APP_ICONS=Icon-29.png Icon-40.png Icon-58.png Icon-80.png Icon-100.png Icon-50.png Icon-72.png Icon-57.png Icon-114.png Icon-76.png Icon-120.png Icon-144.png Icon-152.png
APP_DEFAULT=Default.png Default-568h@2x.png Default@2x.png Default-Landscape@2x~ipad.png Default-Landscape~ipad.png Default-Portrait@2x~ipad.png Default-Portrait~ipad.png 

PNG=$(APP_IMAGES) $(APP_IMAGES_BRANDING) $(APP_ICONS) $(APP_DEFAULT)

all: $(PNG)

32.png: $(ICON)
	rsvg-convert -w 32 -h 32 -f png $< -o $@

128.png: $(ICON)
	rsvg-convert -w 128 -h 128 -f png $< -o $@

ulteo.big.png: $(ICON)
	rsvg-convert -w 188 -h 188 -f png $< -o $@

ulteo_big_logo_text.png: $(LOGO)
	rsvg-convert -w 320 -h 182 -f png $< -o $@

logo_small.png: $(LOGO)
	rsvg-convert -w 116 -h 63 -f png $< -o $@

Icon-%.png: $(ICON)
	size=$(word 2, $(subst -, , $(subst ., ,$@))); \
	let "icon=($$size * 98) / 114"; \
	rsvg-convert -w $$icon -h $$icon -f png $< -o .$@; \
	convert -size $${size}x$$size canvas:black .$@ -gravity center -composite $@ 
	rm .$@

Default.png: $(ICON)
	rsvg-convert -w 114 -h 114 -f png $< -o .$@
	convert -size 320x480 canvas:#dae2e4 .$@ -gravity center -composite $@
	rm .$@

Default-568h@2x.png: $(ICON)
	rsvg-convert -w 228 -h 228 -f png $< -o .$@
	convert -size 640x1136 canvas:#dae2e4 .$@ -gravity center -composite $@
	rm .$@

Default@2x.png: $(ICON)
	rsvg-convert -w 228 -h 228 -f png $< -o .$@
	convert -size 640x960 canvas:#dae2e4 .$@ -gravity center -composite $@
	rm .$@

Default-Landscape@2x~ipad.png: $(ICON)
	rsvg-convert -w 486 -h 486 -f png $< -o .$@
	convert -size 2048x1496 canvas:#dae2e4 .$@ -gravity center -composite $@
	rm .$@

Default-Landscape~ipad.png: $(ICON)
	rsvg-convert -w 243 -h 243 -f png $< -o .$@
	convert -size 1024x748 canvas:#dae2e4 .$@ -gravity center -composite $@
	rm .$@

Default-Portrait@2x~ipad.png: $(ICON)
	rsvg-convert -w 486 -h 486 -f png $< -o .$@
	convert -size 1536x2008 canvas:#dae2e4 .$@ -gravity center -composite $@
	rm .$@

Default-Portrait~ipad.png: $(ICON)
	rsvg-convert -w 243 -h 243 -f png $< -o .$@
	convert -size 768x1004 canvas:#dae2e4 .$@ -gravity center -composite $@
	rm .$@

FloatingToolbarBG.png: FloatingToolbarBG.svgz
	rsvg-convert -f png $< -o $@

FloatingToolbarHelp.png: FloatingToolbarHelp.svgz
	rsvg-convert -f png $< -o $@

FloatingToolbarQuit.png: FloatingToolbarQuit.svgz
	rsvg-convert -f png $< -o $@

toolbar.png: FloatingToolbarQuit.png FloatingToolbarHelp.png FloatingToolbarBG.png
	convert FloatingToolbarBG.png +append -write mpr:sometile +delete -size 198x40 tile:mpr:sometile FloatingToolbarQuit.png -gravity East -composite FloatingToolbarHelp.png -gravity East -geometry +40+0 -composite $@

ulteo-bandeau-light.jpg: ulteo-bandeau-light.svgz
	rsvg-convert -f png $< -o .$@.png
	convert .$@.png $@
	rm .$@.png

clean:
	rm -f $(PNG)
