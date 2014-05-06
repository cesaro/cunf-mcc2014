
# Copyright (C) 2010-2014  Cesar Rodriguez <cesar.rodriguez@cs.ox.ac.uk>
#
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program.  If not, see <http://www.gnu.org/licenses/>.

include defs.mk

.PHONY: fake all g test clean distclean prof dist inst

all: $(TARGETS) tags inst
	./scripts/runit

$(TARGETS) : % : %.o $(OBJS)
	@echo "LD  $@"
	@$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) 

gen :
	xsd cxx-tree \
		--generate-serialization  --generate-doxygen --generate-ostream \
		--generate-comparison  --generate-detach \
		--generate-default-ctor --generate-polymorphic \
		--polymorphic-type-all \
		--namespace-map http://mcc.lip6.fr=mcc  \
		--output-dir src/ --root-element property-set \
		--namespace-map 'http://mcc.lip6.fr/=mcc' \
		doc/mcc-properties.xsd
	mv src/mcc-properties.cxx src/mcc-properties.cc
	mv src/mcc-properties.hxx src/mcc-properties.hh

B=~/BenchKit
C=~/devel/cunf

inst : $(TARGETS)
	cp scripts/BenchKit_head.sh $B
	cd $C; make src/cunf/cunf
	cp $C/src/cunf/cunf $B/bin
	cp src/mcc2cunf $B/bin
	cp $C/tools/cont2pr.pl $B/bin
	cp $C/tools/pnml2pep.py $B/bin
	cp -R $C/tools/ptnet $B/bin/ptnet

fix_namespaces:
	rm -Rf $B/INPUTS/tmp
	mkdir $B/INPUTS/tmp
	cd $B/INPUTS/tmp; \
	set -x; \
	for i in ../*.tgz; do \
		tar xzvf $$i; \
		for j in */*xml; do ~/devel/cunf-mcc2014/mcc14fixnamespace $$j; done; \
		tar czvf $$i *; \
		rm -R $B/INPUTS/tmp/*; \
	done; \
	rm -R $B/INPUTS/tmp

prof : $(TARGETS)
	rm gmon.out.*
	src/main /tmp/ele4.ll_net

tags : $(SRCS)
	ctags -R src

g : $(TARGETS)
	gdb ./src/cunf/cunf

vars :
	@echo CC $(CC)
	@echo CXX $(CXX)
	@echo SRCS $(SRCS)
	@echo MSRCS $(MSRCS)
	@echo OBJS $(OBJS)
	@echo MOBJS $(MOBJS)
	@echo TARGETS $(TARGETS)
	@echo DEPS $(DEPS)

clean :
	@rm -f $(TARGETS) $(MOBJS) $(OBJS)
	@rm -f src/cna/spec_lexer.cc src/cna/spec_parser.cc src/cna/spec_parser.h
	@echo Cleaning done.

distclean : clean
	@rm -f $(DEPS)
	@echo Mr. Proper done.

dist : all
	rm -Rf dist/
	mkdir dist/
	mkdir dist/bin
	mkdir dist/lib
	mkdir dist/examples
	mkdir dist/examples/corbett
	mkdir dist/examples/dekker
	mkdir dist/examples/dijkstra
	cp src/main dist/bin/cunf
	cp src/pep2dot dist/bin
	cp tools/cna dist/bin
	cp tools/grml2pep.py dist/bin
	cp tools/cuf2pep.py dist/bin
	cp minisat/core/minisat dist/bin
	cp -R tools/ptnet dist/lib
	cp -R examples/cont dist/examples/corbett/
	cp -R examples/other dist/examples/corbett/
	cp -R examples/plain dist/examples/corbett/
	cp -R examples/pr dist/examples/corbett/
	for i in 02 04 05 08 10 20 30 40 50; do ./tools/mkdekker.py $$i > dist/examples/dekker/dek$$i.ll_net; done
	for i in 02 03 04 05 06 07; do ./tools/mkdijkstra.py $$i > dist/examples/dijkstra/dij$$i.ll_net; done

-include $(DEPS)

