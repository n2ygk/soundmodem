DOC=selfdocbook
MAIN=$(DOC).sgml
SECTS=intro.sgml tools.sgml structure.sgml tips.sgml source.sgml fdl.sgml
FIGS=simple.fig
JPEGS=$(FIGS:.fig=.jpeg)
EPSS=$(FIGS:.fig=.eps)
GENERATED_IMG=$(JPEGS) $(EPSS)
SRC=$(MAIN) $(SECTS) $(MAIN).sed $(addsuffix .sed,$(SECTS)) Makefile.sed \
	builddate $(GENERATED_IMG)

all: $(DOC).ps $(DOC)

$(DOC).ps: $(SRC)
	db2ps $(MAIN)

$(DOC): $(SRC)
	-$(RM) -r $(DOC)
	db2html $(MAIN)
	(cd $@; for file in *.html; do \
	   mv $$file $$file.tmp;\
	   sed -e 's/&#8212;/--/g' $$file.tmp > $$file;\
	   rm $$file.tmp;\
	 done)
	cp $(JPEGS) $@

# Since this file would be parsed for SGML markup, we have to
# make a safe version (with &, < and > converted to SGML entities)
# In addition, tabs get ignored for some reason, so for the
# Makefile we have to convert them to spaces.
# Also, there seems to be a bug that causes -- to come out as -
# even inside <programlisting> elements, so we work around that.
%.sed: %
	fold -w 72 $< | \
		sed -e "s/$$(echo x | tr x \\t)/        /g" \
		    -e "s/&/\\&amp;/g" \
		    -e "s/</\\&lt;/g" \
		    -e "s/>/\\&gt;/g" \
		    -e "s/--/\\&mdash;/g" > $@

builddate:
	date "+%e %B %Y" > $@

clean:
	-$(RM) *.log *.dvi *.aux *.tex *.sed *.ps builddate
	-$(RM) -r $(DOC) docbook
	-$(RM) $(GENERATED_IMG)

distclean: clean
	-$(RM) *~ selfdocbook.tar.gz docbook.tar.gz

selfdocbook.tar.gz: distclean
	(cd ..; tar zcf /tmp/selfdocbook.tar.gz selfdocbook)
	mv /tmp/selfdocbook.tar.gz .

docbook: all
	-$(RM) -r $@
	mkdir $@
	cp $(DOC).ps $@
	tar cf - $(DOC) | (cd $@; tar xf -)

docbook.tar.gz: selfdocbook.tar.gz docbook
	cp selfdocbook.tar.gz docbook
	tar zcf docbook.tar.gz docbook

# Make encapsulated PostScript from xfig
%.eps: %.fig
	fig2dev -Leps $< $@

# Make jpeg from xfig
%.jpeg: %.fig
	fig2dev -Ljpeg $< $@

.PHONY: distclean clean all builddate
