# -*- coding: utf-8 -*-
# author: examachine aka Eray Ozkural
# a straightforward digraph class using python dicts

import sys
import string

import util

# not an error!

class CycleException(Exception):
    def __init__(self, cycle):
        self.cycle = cycle

    def __str__(self):
        return 'Encountered cycle %s' % self.cycle

class Digraph(object):

    def __init__(self):
        self.__v = set()
        self.__adj = {}
        self.__vdata = {}
        self.__edata = {}
        
    def vertices(self):
        "return set of vertex descriptors"
        return self.__v

    def num_vertices(self):
	return len(self.vertices())

    def edges(self):
        "return a list of edge descriptors"
        list = []
        for u in self.__v:
            for v in self.__adj[u]:
                list.append( (u,v) )
        return list

    def num_edges(self):
	num = 0
	for u in self.vertices():
	    num = num + len(self.adj(u))
	return num

    def from_list(self, el):
        "convert a list of edges (u,v) to graph"
        for (u,v) in el:
            self.add_edge(u,v)

    def add_vertex(self, u, data = None):
        "add vertex u, optionally with data"
        assert not u in self.__v
        self.__v.add(u)
        self.__adj[u] = set()
        if data != None:
            self.__vdata[u] = data
            self.__edata[u] = {}

    def add_edge(self, u, v, edata = None, udata = None, vdata = None):
        "add edge u -> v"
        if not u in self.__v:
            self.add_vertex(u, udata)
        if not v in self.__v:
            self.add_vertex(v, vdata)
	if not v in self.__adj[u]:  
	    self.__adj[u].add(v)
	    if edata != None:
		self.__edata[u][v] = edata

    def add_biedge(self, u, v, edata = None):
        self.add_edge(u, v, edata)
        self.add_edge(v, u, edata)

    def set_vertex_data(self, u, data):
        self.__vdata[u] = data

    def vertex_data(self, u):
        return self.__vdata[u]

    def edge_data(self, u, v):
        return self.__edata[u][v]

    def has_vertex(self, u):
        return u in self.__v

    def has_edge(self, u,v):
        if u in self.__v:
            return v in self.__adj[u]
        else:
            return False

    def adj(self, u):
        return self.__adj[u]

    def dfs(self, finish_hook = None):
        self.color = {}
        self.p = {}
        self.d = {}
        self.f = {}
        for u in self.__v:
            self.color[u] = 'w'         # mark white (unexplored)
            self.p[u] = None
        self.time = 0
        for u in self.__v:
            if self.color[u] == 'w':
                self.dfs_visit(u, finish_hook)

    def dfs_visit(self, u, finish_hook):
        self.color[u] = 'g'             # mark green (discovered)
        self.d[u] = self.time = self.time + 1
        for v in self.adj(u):
            if self.color[v] == 'w':    # explore unexplored vertices
                self.p[v] = u
                self.dfs_visit(v, finish_hook)
            elif self.color[v] == 'g':  # cycle detected
                cycle = [u]
                while self.p[u]:
                    u = self.p[u]
                    cycle.append(u)
                cycle.reverse()
                raise CycleException(cycle)
        self.color[u] = 'b'             # mark black (completed)
        if finish_hook:
            finish_hook(u)
        self.f[u] = self.time = self.time + 1

    def cycle_free(self):
        try:
            self.dfs()
            return True
        except CycleException:
            return False

    def topological_sort(self):
        list = []
        self.dfs(lambda u: list.append(u))
        list.reverse()
        return list

    def id_str(self, u):
        def repl(char):
            if char in string.punctuation:
                return '_'
            else:
                return char
        return util.concat(map(repl, str(u)))

    def write_graphviz(self, f):
        f.write('digraph G {\n')
        for u in self.vertices():
            f.write(self.id_str(u))
            self.write_graphviz_vlabel(f, u)
            f.write(';\n')
        f.write('\n')
        for u in self.vertices():
            for v in self.adj(u):
                f.write( self.id_str(u) + ' -> ' + self.id_str(v))
                self.write_graphviz_elabel(f, u, v)
                f.write(';\n')
        f.write('\n')
        f.write('}\n')

    def write_graphviz_vlabel(self, f, u):
        pass

    def write_graphviz_elabel(self, f, u, v):
        pass

    # assumes vertex data is int
    def write_metis_generic(self, f):
	vertices = self.vertices()	
	num_edges = self.num_edges()/2
	# map vertices to integers starting from 1
	vmap = {}
	i = 1
	for u in vertices:
	    if len(self.adj(u))>0:
		vmap[u] = i
		i = i + 1
	f.write('%d %d 10\n' % (i-1, num_edges))
        for u in vertices:
	    if len(self.adj(u))>0:
		f.write('%d ' % int(self.vertex_data(u)))
		for v in self.adj(u):
		    f.write('%d ' % vmap[v])
                    #self.write_graphviz_elabel(f, u, v)
		f.write('\n')

    # write metis graph with integer vertices starting from 1, 
    # write zero weight vertices for missing vertices
    # add "add" value to vertex indices, default assumes your 
    # vertex indices start from zero, so it adds 1
    def write_metis_intvertices(self, f, add_idx = 1):
	vertices = self.vertices()	
	num_edges = self.num_edges()/2
	vlist = list(vertices)
	vlist.sort()
	max_vertex = max(vlist)
	f.write('%d %d 10\n' % (max_vertex + add_idx, num_edges))
        for u in range(max_vertex + 1):
	    if u in vlist:
		f.write('%d ' % self.vertex_data(u))
		for v in self.adj(u):
                    #print 'writing edge ', u, ' ->', v
		    f.write('%d ' % (v + add_idx))
	    else:
		f.write('0 '); # null vertex
	    f.write('\n')
        f.close()

class Graph(Digraph):

    def add_edge(self, u, v, edata = None, udata = None, vdata = None):
	self.add_biedge(u, v, data, udata, vdata)
