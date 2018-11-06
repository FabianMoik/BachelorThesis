import numpy as np
import random
import csv
import plotly
import plotly.plotly as py
from plotly.offline import download_plotlyjs, init_notebook_mode, plot, iplot
import plotly.graph_objs as go
from plotly import tools

flat_red = '#ef5350'
flat_blue = '#29B6F6'
flat_brown = '#8D6E63'
flat_grey1 = '#757575'
flat_grey2 = '#424242'
flat_grey3 = '#212121'
flat_green = '#388E3C'
flat_orange = '#f0932b'
flat_blue = '#0984e3'


def main():
    #plotly.io.orca.config.executable = '/Users/fabian.moik/anaconda/lib/orca.app/Contents/MacOS/orca'
    #plotly.io.orca.config.save()
    # I need
    #   1.) for agent alone (line diagram)
    #   -   a) average placement over generations                                                                    YES
    #   -   b) hands won over generations                                                                            YES
    #   -   c) mean money over generations  -> a+b+c in one plot                                                     YES
    #   -   overall fitness over generations (once for -> w.o HOF, w. HOF, w. HOF + OPP MODEL)                       NO
    #
    #   2.) for population (histogram)
    #   -   overall fitness of best agent and rest of pop
    #   -   average placement, hands won, mean money of best agent and rest of pop
    smoothness_factor = 50  # value of 50 was used to create the plots for doku
    average_placement, hands_won, mean_money, fitness, vpip, pfr, afq, dollar_won = read_csv()
    average_placement_mean = running_mean(average_placement, smoothness_factor)
    hands_won_mean = running_mean(hands_won, smoothness_factor)
    mean_money_mean = running_mean(mean_money, smoothness_factor)
    fitness_mean = running_mean(fitness, smoothness_factor)
    vpip_mean = running_mean(vpip, smoothness_factor)
    pfr_mean = running_mean(pfr, smoothness_factor)
    afq_mean = running_mean(afq, smoothness_factor)
    dollar_won_mean = running_mean(dollar_won, smoothness_factor)

    print("BEST VALUES:\n")
    print("Best Avg. Place: ", min(average_placement), " at: ", average_placement.index(min(average_placement)))
    print("Best hands won: ", max(hands_won), " at: ", hands_won.index(max(hands_won)))
    print("Best mean money: ", max(mean_money), " at: ", mean_money.index(max(mean_money)))
    print("Best overall Fitness: ", max(fitness), " at: ", fitness.index(max(fitness)))
    print("Best $ won: ", max(dollar_won), " at: ", dollar_won.index(max(dollar_won)))

    best_overall, best_overall_indices = get_best_overall_fitness_indices(fitness, 9)

    print(best_overall, best_overall_indices)

    # WithoutHOF values
    #plot_fitness_histo('histogram', [10.0389, 27.8442, 30.6459, 24.7407, 10.0773],
    #                   [0.5329, 3.502, 3.064, 1.022, 0.9042],
    #                   [870.252, 6351.68, 5296.6, 4977.99, 4847.18], [0.623307, 0.31235, 0.2467, 0.3595, 0.65778])
    #plot_dollar_histo('dollar_roi', [20520.6 / 1000, 14375.5 / 1000, -704.206 / 1000, -73172.2 / 1000, 80265.9 / 1000])

    # WithHOF values
    plot_fitness_histo('histogram', [10.0389, 27.842, 30.581, 24.71, 10.076],
                       [0.5279, 3.484, 3.015, 1.022, 0.984],
                       [871.725, 6371.41, 5352.43, 4972.79, 4983.79],
                       [0.6233, 0.3125, 0.2464, 0.36, 0.6603])
    plot_dollar_histo('dollar_roi', [19524.6 / 1000, 14222.9 / 1000, -833.1 / 1000, -74306.7 / 1000, 93353.3 / 1000])

    # WithHOFAndOPM values
    plot_fitness_histo('histogram', [9.8768, 27.858, 30.583, 24.69, 11.8468],
                     [0.510, 3.462, 2.927, 0.9732, 3.498],
                     [870.771, 6339.58, 5567.8, 5100.59, 5326.81],
                       [0.6266, 0.3118, 0.2481, 0.3614, 0.6312])
    plot_dollar_histo('dollar_roi', [21952.6 / 1000, 14342.9 / 1000, -1015.1 / 1000, -74020.7 / 1000, 82940.3 / 1000])

    #plot_line_evolutionary_progress('evo_progress', average_placement, hands_won, mean_money, fitness)
    #plot_line_evolutionary_progress('evo_progress_smoothed', average_placement_mean, hands_won_mean, mean_money_mean, fitness_mean)

    #plot_overall_fitness('overallfitness', fitness_mean)
    #plot_dollar_won('dollar', dollar_won_mean / 10.0)
    #plot_playing_style_characteristics_progress('stats', vpip_mean, pfr_mean, afq_mean)
    #plot_line_evolutionary_progress_subplots('progression', average_placement_mean, hands_won_mean, mean_money_mean, fitness_mean)


def plot_dollar_histo(name, dollar_won):
    x = ["Folder", "Caller", "Raiser", "Random", "HOF_OPM C."]
    dollar_won_range = 100
    print(dollar_won)
    color1 = flat_green
    color2 = flat_grey2

    data = [
        go.Histogram(
            histfunc="sum",
            y=dollar_won,
            x=x,
            name="Return of Investment (ROI) %",
            marker=dict(color=color1),
            opacity=0.7
        ),
    ]

    layout = go.Layout(
        xaxis=dict(
            domain=[0.1, 0.9],
            title='Agents',
            tickangle=45,
            tickfont=dict(
                size=14,
                color='black'
            )
        ),
        yaxis=dict(
            title='ROI (%)',
            titlefont=dict(
                color=color1
            ),
            tickfont=dict(
                color=color1
            ),
            range=[-dollar_won_range, dollar_won_range]
        ),
        bargap=0.2,
        bargroupgap=0.1
    )

    fig = go.Figure(data=data, layout=layout)
    plotly.io.write_image(fig, '/Users/fabian.moik/Desktop/' + name + '.pdf')
    plot(fig, filename='/Users/fabian.moik/Desktop/styled histogram.html')

def plot_fitness_histo(name, average_placement, hands_won, mean_money, overall_fitness):

    x = ["Folder", "Caller", "Raiser", "Random", "HOF_OPM C."]
    average_placement_range = 35.0
    hands_won_range = 10.0
    mean_money_range = 15000.0
    overall_fitness_range = 0.75
    dollar_won_range = 12000
    scaling_factor = average_placement_range

    color1 = flat_grey1
    color2 = flat_grey2
    color3 = flat_grey3
    color4 = flat_blue
    color5 = flat_orange

    data = [
        go.Histogram(
            histfunc="sum",
            y=average_placement,
            x=x,
            name="Avg. Placement",
            marker=dict(color=color1),
            opacity=0.5
        ),
        go.Histogram(
            histfunc="sum",
            y=[i / (hands_won_range / scaling_factor) for i in hands_won],
            x=x,
            name="Hands won",
            marker=dict(color=color2),
            opacity=0.5
        ),
        go.Histogram(
            histfunc="sum",
            y=[i / (mean_money_range / scaling_factor) for i in mean_money],
            x=x,
            name="Mean Money Won",
            marker=dict(color=color3),
            opacity=0.5
        ),
        go.Histogram(
            histfunc="sum",
            y=[i / (overall_fitness_range / scaling_factor) for i in overall_fitness],
            x=x,
            name="Overall Fitness",
            marker=dict(color=color4),
            opacity=1
        ),
        go.Histogram(histfunc="sum",
            y=[],
            x=x,
            yaxis='y2',
            name="overall fitness",
            marker=dict(color='#ED0503'),
            opacity=1
        ),
        go.Histogram(histfunc="sum",
            y=[],
            x=x,
            yaxis='y3',
            name="overall fitness",
            marker=dict(color='#ED0503'),
            opacity=1
        ),
        go.Histogram(histfunc="sum",
            y=[],
            x=x,
            yaxis='y4',
            name="overall fitness",
            marker=dict(color='#ED0503'),
            opacity=1
        ),
    ]

    layout = go.Layout(
        xaxis=dict(
            domain=[0.1, 0.9],
            title='Agents',
            tickangle=45,
            tickfont=dict(
                size=14,
                color='black'
            )
        ),
        yaxis=dict(
            title='Average Placement in Tournament',
            titlefont=dict(
                color=color1
            ),
            tickfont=dict(
                color=color1
            ),
            range=[0, average_placement_range]
        ),
        yaxis2=dict(
            title='Hands Won / Tournament',
            titlefont=dict(
                color=color2
            ),
            tickfont=dict(
                color=color2
            ),
            anchor='free',
            overlaying='y',
            side='left',
            position=0.02,
            range=[0, hands_won_range]
        ),
        yaxis3=dict(
            title='Mean Money Won',
            titlefont=dict(
                color=color3
            ),
            tickfont=dict(
                color=color3
            ),
            anchor='x',
            overlaying='y',
            side='right',
            range=[0, mean_money_range],
        ),
        yaxis4=dict(
            title='Overall Fitness',
            titlefont=dict(
                color=color4
            ),
            tickfont=dict(
                color=color4
            ),
            anchor='free',
            overlaying='y',
            side='right',
            position=0.98,
            range=[0, overall_fitness_range]
        ),
        bargap=0.2,
        bargroupgap=0.1
    )

    fig = go.Figure(data=data, layout=layout)
    plotly.io.write_image(fig, '/Users/fabian.moik/Desktop/' + name + '.pdf', width=1300, height=600)
    plot(fig, filename='/Users/fabian.moik/Desktop/styled histogram.html')


def plot_line_evolutionary_progress(name, average_place, hands_won, mean_money, fitness):

    N = len(average_place)
    random_x = np.linspace(0, N, N)
    random_y0 = average_place
    random_y1 = hands_won
    random_y2 = mean_money
    random_y3 = fitness

    # Create traces
    trace0 = go.Scatter(
        x=random_x,
        y=random_y0,
        mode='lines',
        name='average placement',
        marker=dict(color='#feca57'),
        opacity=0.5
    )
    trace1 = go.Scatter(
        x=random_x,
        y=random_y1,
        yaxis='y2',
        mode='lines',
        name='hands won',
        marker=dict(color='#c8d6e5'),
        opacity=1
    )
    trace2 = go.Scatter(
        x=random_x,
        y=random_y2,
        yaxis='y3',
        mode='lines',
        name='mean money won',
        marker=dict(color='#8395a7'),
        opacity=1
    )
    trace3 = go.Scatter(
        x=random_x,
        y=random_y3,
        yaxis='y4',
        mode='lines',
        name='overall fitness',
        marker=dict(color='#ee5253'),
        opacity=1,
    )
    data = [trace0, trace1, trace2, trace3]
    layout = go.Layout(
        xaxis=dict(
            domain=[0.1, 0.95]
        ),
        yaxis=dict(
            title='Average Placement in Tournament',
            titlefont=dict(
                color='#feca57'
            ),
            tickfont=dict(
                color='#feca57'
            ),
        ),
        yaxis2=dict(
            title='Hands Won / X Tournaments',
            titlefont=dict(
                color='#c8d6e5'
            ),
            tickfont=dict(
                color='#c8d6e5'
            ),
            anchor='free',
            overlaying='y',
            side='left',
            position=0.05,
            range=[0, 1000]
        ),
        yaxis3=dict(
            title='Mean Money Won',
            titlefont=dict(
                color='#8395a7'
            ),
            tickfont=dict(
                color='#8395a7'
            ),
            anchor='x',
            overlaying='y',
            side='right',
            range=[100, 2000],
        ),
        yaxis4=dict(
            title='Overall Fitness (smaller is better)',
            titlefont=dict(
                color='#ee5253'
            ),
            tickfont=dict(
                color='#ee5253'
            ),
            anchor='free',
            overlaying='y',
            side='right',
            position=1,
        ),
    )

    fig = go.Figure(data=data, layout=layout)
    plotly.io.write_image(fig, '/Users/fabian.moik/Desktop/' + name + '.pdf', width=1000)
    plot(fig, filename='/Users/fabian.moik/Desktop/'+name+'.html')


def plot_line_evolutionary_progress_subplots(name, average_place, hands_won, mean_money, fitness):

    N = len(average_place)
    random_x = np.linspace(0, N, N)
    random_y0 = average_place
    random_y1 = hands_won
    random_y2 = mean_money
    random_y3 = fitness

    color1 = flat_blue
    color2 = flat_grey1
    color3 = flat_grey3

    # Create traces
    trace0 = go.Scatter(
        x=random_x,
        y=random_y0,
        mode='lines',
        name='Average Placement',
        marker=dict(color=color1),
        opacity=0.85
    )
    trace1 = go.Scatter(
        x=random_x,
        y=random_y1,
        yaxis='y2',
        mode='lines',
        name='Hands Won',
        marker=dict(color=color2),
        opacity=0.8
    )
    trace2 = go.Scatter(
        x=random_x,
        y=random_y2,
        yaxis='y3',
        mode='lines',
        name='Mean Money Won',
        marker=dict(color=color3),
        opacity=0.8
    )

    data = [trace0, trace1, trace2]
    layout = go.Layout(
        xaxis=dict(
            title ='Generations',
            domain=[0.07, 1]
        ),

        yaxis=dict(
            title='Average Placement in Tournament',
            titlefont=dict(
                color=color1
            ),
            tickfont=dict(
                color=color1
            ),
            position=0.06,
            domain=[0, 0.33]
        ),
        yaxis2=dict(
            title='Hands Won / Tournament',
            titlefont=dict(
                color=color2
            ),
            tickfont=dict(
                color=color2
            ),
            anchor='x2',
            position=0.03,
            domain=[0.34, 0.67]
        ),
        yaxis3=dict(
            title='Mean Money Won',
            titlefont=dict(
                color=color3
            ),
            tickfont=dict(
                color=color3
            ),
            position=0.06,
            domain=[0.68, 1]
        )
    )
    '''
        yaxis4=dict(
            title='Overall Fitness (smaller is better)',
            titlefont=dict(
                color='#ee5253'
            ),
            tickfont=dict(
                color='#ee5253'
            ),
            anchor='x4',
            position=0.01,
            domain=[0.51, 1]
        ),
    '''

    fig = go.Figure(data=data, layout=layout)
    #plotly.io.write_image(fig, file='/Users/fabian.moik/Desktop/'+name, format='png')
    #iplot(fig, filename='/Users/fabian.moik/Desktop/'+name, image='svg')
    plotly.io.write_image(fig, '/Users/fabian.moik/Desktop/' + name + '.pdf', width=1300, height=800)
    plot(fig, filename='/Users/fabian.moik/Desktop/'+name+'.html')


def plot_overall_fitness(name, fitness):
    N = len(fitness)
    random_x = np.linspace(0, N, N)
    random_y0 = fitness
    color = flat_blue

    # Create traces
    trace0 = go.Scatter(
        x=random_x,
        y=random_y0,
        mode='lines',
        name='overall fitness',
        marker=dict(color=color),
        opacity=1
    )

    data = [trace0]
    layout = go.Layout(
        xaxis=dict(
            title='Generations',
            domain=[0.02, 1]
        ),

        yaxis=dict(
            title='Overall Fitness',
            titlefont=dict(
                color=color
            ),
            tickfont=dict(
                color=color
            ),
            position=0,
        ),
    )

    fig = go.Figure(data=data, layout=layout)
    plotly.io.write_image(fig, '/Users/fabian.moik/Desktop/' + name + '.pdf', width=1000)
    plot(fig, filename='/Users/fabian.moik/Desktop/' + name + '.html')


def plot_dollar_won(name, dollar_won):
    N = len(dollar_won)
    random_x = np.linspace(0, N, N)
    random_y0 = dollar_won

    color = flat_green

    # Create traces
    trace0 = go.Scatter(
        x=random_x,
        y=random_y0,
        mode='lines',
        name='Mean Dollar won',
        marker=dict(color=color),
        opacity=1
    )

    data = [trace0]
    layout = go.Layout(
        xaxis=dict(
            title='Generations',
            domain=[0.02, 1]
        ),

        yaxis=dict(
            title='ROI (%)',
            titlefont=dict(
                color=color
            ),
            tickfont=dict(
                color=color
            ),
            position=0,
        ),
    )

    fig = go.Figure(data=data, layout=layout)
    plotly.io.write_image(fig, '/Users/fabian.moik/Desktop/' + name + '.pdf', width=1000)
    plot(fig, filename='/Users/fabian.moik/Desktop/' + name + '.html')


def plot_playing_style_characteristics_progress(name, vpip, pfr, afq):
    N = len(vpip)
    random_x = np.linspace(0, N, N)
    random_y0 = vpip
    random_y1 = pfr
    random_y2 = afq

    color1 = flat_grey1
    color2 = flat_grey3
    color3 = flat_blue

    # Create traces
    trace0 = go.Scatter(
        x=random_x,
        y=random_y0,
        mode='lines',
        name='VPIP (%)',
        marker=dict(color=color1),
        opacity=0.8
    )
    trace1 = go.Scatter(
        x=random_x,
        y=random_y1,
        yaxis='y2',
        mode='lines',
        name='PFR (%)',
        marker=dict(color=color2),
        opacity=0.8
    )
    trace2 = go.Scatter(
        x=random_x,
        y=random_y2,
        yaxis='y3',
        mode='lines',
        name='AFQ overall (%)',
        marker=dict(color=color3),
        opacity=0.8
    )

    data = [trace0, trace1, trace2]

    layout = go.Layout(
        xaxis=dict(
            title='Generations',
            domain=[0.1, 0.95]
        ),
        yaxis=dict(
            title='VPIP (Voluntarily Put Money In Pot %)',
            titlefont=dict(
                color=color1
            ),
            tickfont=dict(
                color=color1
            ),
            range=[0, 40]
        ),
        yaxis2=dict(
            title='PFR (Preflop Raise %)',
            titlefont=dict(
                color=color2
            ),
            tickfont=dict(
                color=color2
            ),
            anchor='free',
            overlaying='y',
            side='left',
            range=[0, 40],
            position=0.05
        ),
        yaxis3=dict(
            title='Overall Aggression Frequency in %',
            titlefont=dict(
                color=color3
            ),
            tickfont=dict(
                color=color3
            ),
            anchor='x',
            overlaying='y',
            side='right',
            range=[0, 40]
        ),
    )

    fig = go.Figure(data=data, layout=layout)
    plotly.io.write_image(fig, '/Users/fabian.moik/Desktop/' + name + '.pdf', width=1300, height=700)
    plot(fig, filename='/Users/fabian.moik/Desktop/' + name + '.html')


def read_csv():
    average_placement = []
    hands_won = []
    mean_money = []
    fitness = []
    vpip = []
    pfr = []
    afq = []
    dollar_won = []
    with open('results.csv') as csvfile:
        readCSV = csv.reader(csvfile, delimiter=',')
        for row in readCSV:
            if row[0] != 'gens':
                average_placement.append(float(row[1]))
                hands_won.append(float(row[2]))
                mean_money.append(float(row[3]))
                fitness.append(float(row[4]))
                vpip.append(float(row[5]))
                pfr.append(float(row[6]))
                afq.append(float(row[7]))
                dollar_won.append(float(row[8]))

    return average_placement, hands_won, mean_money, fitness, vpip, pfr, afq, dollar_won


def running_mean(x, N):
    cumsum = np.cumsum(np.insert(x, 0, 0))
    return (cumsum[N:] - cumsum[:-N]) / float(N)


def get_best_overall_fitness_indices(fitness, index):
    sorted_fitness = sorted(fitness)[len(fitness) - index:len(fitness)]
    sorted_fitness_indices = []

    for fit in sorted_fitness:
        sorted_fitness_indices.append(fitness.index(fit))

    return sorted_fitness, sorted_fitness_indices


if __name__ == '__main__':
    main()

