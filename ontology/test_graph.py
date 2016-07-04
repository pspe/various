

import pygraphviz as pgv



g = pgv.AGraph(directed=True, strict=False)
g.graph_attr['label'] = 'test'

g.add_node ('START', shape='circle', color='red', style='filled', fill='red', fontcolor='white', )
g.add_node ('A', shape='circle', color='blue', style='filled', fill='red', fontcolor='white', )
g.add_node ('B', shape='circle', color='blue', style='filled', fill='red', fontcolor='white', )
g.add_node ('C', shape='circle', color='blue', style='filled', fill='red', fontcolor='white', )


g.add_edge ('START', 'A', color='black', label = '1', labelfontcolor='black', labelfontsize=8)
g.add_edge ('START', 'A', label = '2', labelfontcolor='red', labelfontsize=8)
g.add_edge ('START', 'A', label = '3', labelfontcolor='blue', labelfontsize=8)
g.add_edge ('A', 'B', color='red', label = '4', labelfontcolor='green', labelfontsize=8)
g.add_edge ('A', 'B', color='black', label = '5', fontcolor='green', labelfontsize=8)
g.add_edge ('B', 'C', color='green', label = '6', labelfontcolor='gray', labelfontsize=8)
g.add_edge ('C', 'C', color='black', label = '7', labelfontcolor='gray', labelfontsize=8)



g.layout(prog='dot')
#g.layout(prog='circo')
g.draw(g.graph_attr['label']+'.png')

