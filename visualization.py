# ﷽
# 与可视化相关的代码

from random import random

import folium
from haversine import haversine, Unit
from more_itertools import flatten
from pyecharts import options as opts
from pyecharts.charts import Graph


# 将HSV值转换成RGB值
# 来源：https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
# HSV values in [0..1]
# returns [r, g, b] values from 0 to 255
def hsv_to_rgb(h, s, v):
    h_i = int(h * 6)
    f = h * 6 - h_i
    p = v * (1 - s)
    q = v * (1 - f * s)
    t = v * (1 - (1 - f) * s)
    if h_i == 0:
        r, g, b = v, t, p
    elif h_i == 1:
        r, g, b = q, v, p
    elif h_i == 2:
        r, g, b = p, v, t
    elif h_i == 3:
        r, g, b = p, q, v
    elif h_i == 4:
        r, g, b = t, p, v
    elif h_i == 5:
        r, g, b = v, p, q
    r *= 256
    g *= 256
    b *= 256
    return int(r) if r <= 255 else 255, int(g) if g <= 255 else 255, int(b) if b <= 255 else 255

# 生成一组随机颜色（生成器）
# 参考：https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
# 每次返回一个(r, g, b)三元组
# n为随机颜色的个数
# saturation为颜色的饱和度，范围为0~1
# value为颜色的明度，范围为0~1
def generate_random_colors(n, saturation=0.75, value=0.75):
    # use golden ratio
    golden_ratio_conjugate = 0.618033988749895

    hue = random()

    for i in range(n):
        hue += golden_ratio_conjugate
        hue %= 1
        yield hsv_to_rgb(hue, saturation, value)

# 将RGB颜色转换为hex编码
def to_hex_code(r, g, b):
    return '#%02x%02x%02x' % (r, g, b)

def get_bounds(points):
    min_longitude = 180
    max_longitude = -180
    min_latitude = 90
    max_latitude = -90
    
    for (latitude, longitude) in points:
        if latitude < min_latitude:
            min_latitude = latitude
        if latitude > max_latitude:
            max_latitude = latitude
        if longitude < min_longitude:
            min_longitude = longitude
        if longitude > max_longitude:
            max_longitude = longitude
    
    return [(min_latitude, min_longitude), (max_latitude, max_longitude)]

# 利用pyecharts，将社交网络可视化，并突出强调其中的社区，最后返回可视化对象
# 通过visualize_social_network_and_emphasize_communities(...).render_notebook()在Jupyter Notebook内渲染可视化对象
# social_network是一个nx.Graph对象
# communities是一个集合的列表，每个集合代表一个社区的结点
# 可选参数community_names是一个字符串的列表，每个字符串代表相应社区的名称
# 如果不提供，那么各社区的名称将会是'0', '1', '2', ...
# 可选参数default_community_name是一个字符串，代表不在社区中的结点的“社区名称”
# 如果不提供，默认值为'default'
def visualize_social_network_and_emphasize_communities(social_network, communities=[], community_names=[], default_community_name='default', **drawing_kwargs):
    
    # 剩下来的结点
    remaining_nodes = set(social_network.nodes)
    
    # ECharts关系图的边列表
    links = [
        {
            # source、target属性必须为字符串
            'source': str(source),
            'target': str(target),
        }
        for (source, target) in social_network.edges
    ]
    
    # ECharts关系图的结点列表和分类列表
    # 结点列表的格式为[{'name': 'Myriel', 'category': 0}, ...]
    # 分类列表的格式为[{'name': '类目0'}, ...]
    nodes = []
    categories = []
    
    # 遍历所有的社区和社区名称（如果有的话）
    if community_names:
        for (i, (community, community_name)) in enumerate(zip(communities, community_names)):
            for node in community:
                # 添加结点
                nodes.append({'name': str(node), 'category': i})
                # 更新"剩下来的结点"
                remaining_nodes.discard(node)
            
            # 添加一个分类，分类名称就是社区名称
            categories.append({'name': community_name})

    # 没有社区名称，那么各社区的名称将会是'0', '1', '2', ...
    else:
        for (i, community) in enumerate(communities):
            for node in community:
                # 添加结点
                nodes.append({'name': str(node), 'category': i})
                # 更新"剩下来的结点"
                remaining_nodes.discard(node)
            
            # 添加一个分类，分类名称就是社区名称
            categories.append({'name': str(i)})
    
    # 添加"剩下来的结点"
    nodes.extend(({'name': str(node), 'category': len(categories)} for node in remaining_nodes))
    categories.append({'name': default_community_name})
    
    # 创建可视化
    visualization = (
        Graph()
        .add(
            series_name='',
            nodes=nodes,
            links=links,
            categories=categories,
            **drawing_kwargs
        )
    )
    
    return visualization


# 利用pyecharts，将多级社交网络可视化，最后返回可视化对象
def visualize_multiplex_social_network(vertices, edges_list=[], edges_line_styles=[]):
    
    # 剩下来的结点
    remaining_nodes = set(vertices)
    
    # ECharts关系图的边列表
    links = []
    for (edges, edges_line_style) in zip(edges_list, edges_line_styles):
        for (source, target) in edges:
            links.append(opts.GraphLink(source=str(source), target=str(target), linestyle_opts=edges_line_style))
    
    # ECharts关系图的结点列表
    # 结点列表的格式为[{'name': 'Myriel', 'category': 0}, ...]
    nodes = []
    
    for vertex in vertices:
        nodes.append({'name': str(vertex)})
    
    # 创建可视化
    visualization = (
        Graph()
        .add(
            series_name='',
            nodes=nodes,
            links=links,
        )
    )
    
    return visualization


# 利用folium，将轨迹可视化，最后返回folium.Map对象
# 通过回显folium.Map对象在Jupyter Notebook内渲染可视化对象
# 参考https://www.analyticsvidhya.com/blog/2020/06/guide-geospatial-analysis-folium-python/
# trajectories是一个轨迹的列表
# 其中每条轨迹的格式如下：
# [[纬度1, 经度1], [纬度2, 经度2], ...]
# 可选参数trajectory_names是一个字符串的列表，每个字符串代表相应轨迹的名称
# 如果不提供，那么各轨迹的名称将会是'0', '1', '2', ...
def visualize_trajectories(trajectories=None, trajectory_names=None):
    # 返回的地图对象
    folium_map = folium.Map()
    
    # 判断轨迹是否为空
    if trajectories:
        folium_map.fit_bounds(get_bounds(flatten(trajectories)))
        
        # 创建包含轨迹的特征组
        feature_group = folium.FeatureGroup('Trajectories')
        
        # 随机颜色生成器
        random_color_generator = generate_random_colors(n=len(trajectories))
        
        # 遍历所有的轨迹名称（如果有的话）
        # 没有轨迹名称，那么各轨迹的名称将会是'0', '1', '2', ...
        if trajectory_names is None:
            trajectory_names = [str(i) for (i, trajectory) in enumerate(trajectories)] 
        
        for (i, (trajectory, trajectory_name)) in enumerate(zip(trajectories, trajectory_names)):
            # 获取一个随机颜色的hex码
            random_color_hex_code = to_hex_code(*next(random_color_generator))
            # 将轨迹名称加入到提示框中
            tooltip = '<span style="color: %s">%s</span>' % (random_color_hex_code, trajectory_name)
            # 将轨迹加入到特征组上
            line = folium.vector_layers.PolyLine(trajectory, color=random_color_hex_code, tooltip=tooltip, popup=tooltip)
            line.add_to(feature_group)

        # 添加包含轨迹的特征组
        feature_group.add_to(folium_map)
    
    # 返回地图对象
    return folium_map

def visualize_points(points=None, point_names=None):
    # 返回的地图对象
    folium_map = folium.Map()
    
    if points:
        folium_map.fit_bounds(get_bounds(points))
        
        # 创建特征组
        feature_group = folium.FeatureGroup('Points')
        
        # 随机颜色生成器
        random_color_generator = generate_random_colors(n=len(points))
        
        # 遍历所有的名称（如果有的话）
        # 没有名称，那么各轨迹的名称将会是'0', '1', '2', ...
        if point_names is None:
            point_names = [str(i) for (i, point) in enumerate(points)] 
        
        for (i, (point, point_name)) in enumerate(zip(points, point_names)):
            # 获取一个随机颜色的hex码
            random_color_hex_code = to_hex_code(*next(random_color_generator))
            # 将名称加入到提示框中
            tooltip = '<span style="color: %s">%s</span>' % (random_color_hex_code, point_name)
            # 加入到特征组上
            marker = folium.Marker(point, color=random_color_hex_code, tooltip=tooltip, popup=tooltip)
            marker.add_to(feature_group)

        # 添加特征组
        feature_group.add_to(folium_map)
    
    # 返回地图对象
    return folium_map

