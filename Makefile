SOURCE_PATH=source

generate-resources: $(SOURCE_PATH)/lexems.l $(SOURCE_PATH)/parser.y
	flex $(SOURCE_PATH)/lexems.l
	bison -d -t $(SOURCE_PATH)/parser.y
	echo '#include "node.h"' | cat - parser.tab.h > temp && mv temp parser.tab.h

