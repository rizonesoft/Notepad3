HTML=$(patsubst %.txt,%.html,$(wildcard *.txt))

all: $(HTML)
  
# when each target of a multi-target rule has its own prereq 
# we use a static pattern rule. 
$(HTML): %.html: %.txt
	asciidoc -a toc2 $<

TMP=/tmp/uthash-gh-pages
stage:
	mkdir -p ${TMP}
	rm -if ${TMP}/*
	cp *.html *.css *.png ${TMP}

.PHONY: clean
clean:
	$(RM) $(HTML)
