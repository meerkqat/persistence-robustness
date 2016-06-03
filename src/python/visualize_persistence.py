'''
Persistence visualizer. Much visual. Very persistence.
'''


import matplotlib.pyplot as plt


def max12(arr):
    m0_i = m1_i = -1
    m0 = m1 = float('-inf')
    c = 0

    for i, x in enumerate(arr):
        if x >= m0:
            m0, m1 = x, m0
            m0_i, m1_i = i, m0_i
        elif x > m1:
            m1 = x
            m1_i = i

        c += 1

    return m0, m0_i, m1, m1_i if c > 1 else None


def parse(file_name, min_delta, cut_distance=10):
    data = {0: [], 1: [], 2: []}

    with open(file_name) as file:
        for line in file:
            betti, start, end = line.strip().split(' ')

            if abs(float(end) - float(start)) > min_delta:
                data[int(betti)] += [(float(start), float(end))]

    '''
    for k in data:
        m0, m0_i, m1, m1_i = max12([t[1] for t in data[k]])
        if abs(m0 - m1) > cut_distance:
            s, _ = data[k][m0_i]
            data[k][m0_i] = (s, m1 + cut_distance)
    '''

    data[0][0] = (0, max(max12([d[1] for d in data[0]])[2], max(max([d[1] for d in data[1]]), max([d[1] for d in data[2]]))) + cut_distance)

    return data


def visualize(data):

    fig = plt.figure()
    fig.suptitle('Persistence diagram')
    hax0 = fig.add_subplot(311)
    hax1 = fig.add_subplot(312, sharex=hax0)
    hax2 = fig.add_subplot(313, sharex=hax1)

    betti = 0
    limit = len(data[betti])

    print len(data[0]), len(data[1]), len(data[2])
    print [t[0] for t in data[betti]][0:limit]
    print [t[1] for t in data[betti]][0:limit]

    hax0.hlines(range(limit), [t[0] for t in data[betti]][0:limit], [t[1] for t in data[betti]][0:limit], colors='g', lw=3)
    #hax0.set_xlabel('Eps')
    hax0.set_ylabel('H0')
    hax0.yaxis.set_visible(False)
    hax0.xaxis.set_visible(False)

    betti = 1
    limit = len(data[betti])
    hax1.hlines(range(limit), [t[0] for t in data[betti]][0:limit], [t[1] for t in data[betti]][0:limit], colors='r', lw=3)
    hax1.set_ylabel('H1')
    hax1.yaxis.set_visible(False)
    hax1.xaxis.set_visible(False)

    betti = 2
    limit = len(data[betti])
    hax2.hlines(range(limit), [t[0] for t in data[betti]][0:limit], [t[1] for t in data[betti]][0:limit], colors='b', lw=3)
    hax2.set_ylabel('H2')
    hax2.yaxis.set_visible(False)

    fig.tight_layout(h_pad=0)
    #x0, x1, y0, y1 = plt.axis()
    #plt.axis((x0, x1, y0 + 100, y1)) # ???
    plt.savefig('persistence_diagram_data1.png')
    plt.show()



def run():
    file_name = "persistence_diagram_data.txt"
    data = parse(file_name, 0.1)
    visualize(data)


# run stuff
run()
