SVG=$(wildcard *.svgz)
DST_XHDPI=../res/drawable-xhdpi
DST_HDPI=../res/drawable-hdpi
DST_MDPI=../res/drawable-mdpi
DST_LDPI=../res/drawable-ldpi
DIRS=$(DST_XHDPI) $(DST_HDPI) $(DST_MDPI) $(DST_LDPI)
PNG=$(foreach DIR, $(DIRS), $(addprefix $(DIR)/, $(patsubst %.svgz,%.png,$(SVG))))
PNG+=$(foreach DIR, $(DIRS), $(addprefix $(DIR)/, icon_bw.png))
BIN=../bin
MARKET=$(patsubst market/%.svgz, $(BIN)/%.png, $(wildcard market/*.svgz))

all: $(BIN) $(DIRS) $(PNG) $(MARKET)

$(BIN) $(DIRS):
	mkdir $@

$(DST_XHDPI)/%.png: %.svgz
	rsvg-convert -z 3.555 -f png $< -o $@

$(DST_HDPI)/%.png: %.svgz
	rsvg-convert -z 2.666 -f png $< -o $@

$(DST_MDPI)/%.png: %.svgz
	rsvg-convert -z 1.777 -f png $< -o $@

$(DST_LDPI)/%.png: %.svgz
	rsvg-convert -z 1.333 -f png $< -o $@

$(BIN)/%.png: market/%.svgz
	rsvg-convert -f png $< -o $@

%/icon_bw.png: %/icon.png
	convert $< -alpha on +opaque black -resize 50% $@

clean:
	rm -f $(PNG) $(MARKET) 
