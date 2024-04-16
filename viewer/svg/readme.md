[mdn.svg](https://developer.mozilla.org/zh-CN/docs/Web/SVG)

SVG 诞生于 1999 年。加载慢是 SVG 的一个缺点。但是 SVG 也有自身的优点，比如它实现了 DOM 接口。


SVG 也提供了一些元素，用于定义圆形、矩形、简单或复杂的曲线。SVG 支持渐变、旋转、动画、滤镜效果、与 JavaScript 交互等等功能

**需要主要的两点**
- SVG 的元素和属性必须按标准格式书写，因为 XML 是区分大小写的（这一点和 HTML 不同）
- SVG 里的属性值必须用引号引起来，就算是数值也必须这样做。



**坐标系统**
以页面的左上角为 (0,0) 坐标点，坐标以像素为单位，x 轴正方向是向右，y 轴正方向是向下


用户单位和屏幕单位的映射关系被称为**用户坐标系统**。

坐标系统还可以旋转、倾斜、翻转。


```
<svg width="200" height="200" viewBox="0 0 100 100">…</svg>
```

viewBox 属性定义了 SVG 的可视区域，它必须包含在 SVG 元素中。 相当于放大了2倍


**基本形状**

基本形状有：正方形、圆、椭圆、线、折线、多边形、Path等


Ellipse 是circle元素更通用的形式，你可以分别缩放圆的 x 半径和 y 半径，将圆变成椭圆。


polygon的路径在最后一个点处自动回到第一个点。


path比较复杂，可以绘制矩形、圆、椭圆、折线、多边形、曲线等。


**path**

path 元素的形状是通过属性 d 定义的，属性d的值是一个“命令 + 参数”的序列.

每一个命令都用一个关键字母来表示.

每一个命令都有两种表示方式，一种是用大写字母，表示采用绝对定位。另一种是用小写字母，表示采用相对定位（例如：从上一个点开始，向上移动 10px，向左移动 7px）

因为属性 d采用的是用户坐标系统，所以不需标明单位。

M 命令经常出现在路径的开始处，用来指明从何处开始画。

H 绘制水平线

V 绘制垂直线

L 在当前位置和新位置之间话一条线段

Z 从当前点画一条直线到路径的起点，命令不用区分大小写。

绘制平滑曲线的命令有三个，其中两个用来绘制贝塞尔曲线，另外一个用来绘制弧形或者说是圆的一部分。

C 三次贝塞尔曲线

三次贝塞尔曲线需要定义一个点和两个控制点

最后一个坐标 (x,y) 表示的是曲线的终点，另外两个坐标是控制点，(x1,y1) 是起点的控制点，(x2,y2)

S 延长三次贝塞尔曲

如果 S 命令跟在一个 C 或 S 命令后面，则它的第一个控制点会被假设成前一个命令曲线的第二个控制点的中心对称点。

如果 S 命令单独使用，前面没有 C 或 S 命令，那当前点将作为第一个控制点。

Q 二次贝塞尔曲线

两组参数，控制点和终点坐标。

T 延长二次贝塞尔曲线

虽然三次贝塞尔曲线拥有更大的自由度，但是两种曲线能达到的效果总是差不多的。具体使用哪种曲线，通常取决于需求，以及对曲线对称性的依赖程度。


A 绘制圆弧

已知椭圆形的长轴半径和短轴半径，并且已知两个点（在椭圆上），根据半径和两点，可以画出两个椭圆，在每个椭圆上根据两点都可以画出两种弧形。

所以，仅仅根据半径和两点，可以画出四种弧形。

两点连线（也就是对角线）正好穿过了椭圆的中心。 只有种弧形可以选择
```
 A rx ry x-axis-rotation large-arc-flag sweep-flag x y
 a rx ry x-axis-rotation large-arc-flag sweep-flag dx dy
```

rx ry 是圆弧的x轴半径,y轴半径，x-axis-rotation 是圆弧的旋转角度，large-arc-flag 是大弧标志，sweep-flag 是顺时针标志。

large-arc-flag 决定弧线是大于还是小于 180 度，0 表示小角度弧，1 表示大角度弧。

sweep-flag 表示弧线的方向，0 表示从起点到终点沿逆时针画弧，1 表示从起点到终点沿顺时针画弧。

用路径来绘制完整的圆或者椭圆是比较困难的，因为圆上的任意点都可以是起点同时也是终点，无数种方案可以选择，真正的路径无法定义。通过绘制连续的路径段落，也可以达到近似的效果，但使用真正的 circle 或者 ellipse 元素会更容易一些。

**Fill 和 Stroke 属性**

fill属性设置对象内部的颜色
stroke属性设置绘制对象的线条的颜色


fill-opacity控制填充色的不透明度
stroke-opacity控制描边的不透明度

stroke-width属性定义了描边的宽度。注意，描边是以路径为中心线绘制的


stroke-linecap属性的值有三种可能值：

- butt用直边结束线段，它是常规做法，线段边界 90 度垂直于描边的方向、贯穿它的终点。
- square的效果差不多，但是会稍微超出实际路径的范围，超出的大小由stroke-width控制。
- round表示边框的终点是圆角，圆角的半径也是由stroke-width控制的。


stroke-linejoin

- miter是默认值，表示用方形画笔在连接处形成尖角
- round表示用圆角连接，实现平滑效果
- bevel，连接处会形成一个斜接

stroke-dasharray

一组用逗号分割的数字组成的数列,必须用逗号分割（空格会被忽略）.

每一组数字，第一个用来表示填色区域的长度，第二个用来表示非填色区域的长度。

fill-rule，用于定义如何给图形重叠的区域上色；

stroke-miterlimit，定义什么情况下绘制或不绘制边框连接的miter效果；

还有stroke-dashoffset，定义虚线开始的位置。

除了定义对象的属性外，你也可以通过 CSS 来样式化填充和描边。

注意，不是所有的属性都能用 CSS 来设置。上色和填充的部分一般是可以用 CSS 来设置的，比如fill，stroke，stroke-dasharray等，但是不包括下面会提到的渐变和图案等功能。另外，width、height，以及路径的命令等等，都不能用 CSS 设置。判断它们能不能用 CSS 设置还是比较容易的。

[SVG 规范](https://www.w3.org/TR/SVG/propidx.html)将属性区分成 properties（属性） 和其他 attributes(特性），前者是可以用 CSS 设置的，后者不能。

**渐变**

```
<stop offset="100%" stop-color="yellow" stop-opacity="0.5" />
```
offset指定结束的位置
stop-color指定颜色
stop-opacity来设置某个位置的半透明度

```
<linearGradient id="Gradient2" x1="0" x2="0" y1="0" y2="1"></linearGradient>
```

渐变的方向可以通过两个点来控制，它们分别是属性 x1、x2、y1 和 y2，这些属性定义了渐变路线走向。渐变色默认是水平方向的，但是通过修改这些属性，就可以旋转该方向。

水平方向渐变分为从左到右、从右到左， 垂直方向渐变分为从上到下、 从下到上。


xlink:href ==  fill="url(#Gradient2)" 


**径向渐变**

中心

渐变结束所围绕的圆环，由 cx 和 cy 属性及半径 r 来定义，通过设置这些点我们可以移动渐变范围并改变它的大小

焦点

描述了渐变的中心，由 fx 和 fy 属性定义。


spreadMethod

控制了当渐变到达终点的行为，但是此时该对象尚未被填充颜色。

Pad 当渐变到达终点时，最终的偏移颜色被用于填充对象剩下的空间

reflect 会让渐变一直持续下去，不过它的效果是与渐变本身是相反的，以 100% 偏移位置的颜色开始，逐渐偏移到 0% 位置的颜色，然后再回到 100% 偏移位置的颜色。

repeat 也会让渐变继续，但是它不会像 reflect 那样反向渐变，而是跳回到最初的颜色然后继续渐变。


gradientUnits

描述了用来描述渐变的大小和方向的单元系统。

该属性有两个值：userSpaceOnUse 、objectBoundingBox。

默认值为 objectBoundingBox，它大体上定义了对象的渐变大小范围，所以你只要指定从 0 到 1 的坐标值，渐变就会自动的缩放到对象相同大小。

userSpaceOnUse 使用绝对单元，所以你必须知道对象的位置，并将渐变放在同样地位置上

## text
属性 x 和属性 y 性决定了文本在视口中显示的位置。属性text-anchor，可以有这些值：start、middle、end 或 inherit，允许决定从这一点开始的文本流的方向。



**设置字体属性**

font-family、font-style、font-weight、font-variant、font-stretch、font-size、font-size-adjust、kerning、letter-spacing、word-spacing和text-decoration。

**tspan**
该元素用来标记大块文本的子部分，它必须是一个text元素或别的tspan元素的子元素

tspan元素有以下的自定义属性：

x 为容器设置一个新绝对x坐标。它覆盖了默认的当前的文本位置。这个属性可以包含一个数列，它们将一个一个地应用到tspan元素内的每一个字符上。

dx 从当前位置，用一个水平偏移开始绘制文本。这里，你可以提供一个值数列，可以应用到连续的字体，因此每次累积一个偏移。


此外还有属性y和属性dy作垂直转换。

rotate 把所有的字符旋转一个角度。如果是一个数列，则使每个字符旋转分别旋转到那个值，剩下的字符根据最后一个值旋转。

textLength 这是一个很模糊的属性，给出字符串的计算长度。它意味着如果它自己的度量文字和长度不满足这个提供的值，则允许渲染引擎精细调整字型的位置。

**tref**
tref元素允许引用已经定义的文本，高效地把它复制到当前位置。你可以使用xlink:href属性，把它指向一个元素，取得其文本内容。你可以独立于源样式化它、修改它的外观。


**textPath**
该元素利用它的xlink:href属性取得一个任意路径，把字符对齐到路径，于是字体会环绕路径、顺着路径走

## 基础变形

<g> 把属性赋给一整个元素集合

平移  translate()

旋转 rotate()

斜切 skewX() skewY()

缩放 scale()


如果使用了变形，你会在元素内部建立了一个新的坐标系统，应用了这些变形，你为该元素和它的子元素指定的单位可能不是 1:1 像素映射。但是依然会根据这个变形进行歪曲、斜切、转换、缩放操作。


## 剪切

Clipping 用来移除在别处定义的元素的部分内容。在这里，任何半透明效果都是不行的。它只能要么显示要么不显示。

Masking 允许使用透明度和灰度值遮罩计算得的软边缘



opacity 设置整个元素的透明度， fill-opacity和stroke-opacity，分别用来控制填充和描边的不透明度

需要注意的是描边将绘制在填充的上面。因此，如果你在一个元素上设置了描边透明度，但它同时设有填充，则描边的一半应用填充色，另一半将应用背景色。


## 嵌入光栅图像

嵌入的图像变成一个普通的 SVG 元素。这意味着，你可以在其内容上用剪切、遮罩、滤镜、旋转以及其他 SVG 工具

## 嵌入任意 XML
可以在 SVG 文档的任何位置嵌入任意 XML。


foreignObject 用来在 SVG 中嵌入 XHTML。另一个经常被引用的用例是用 **MathML** 写的方程式。


## 滤镜（Filter）就是 SVG 中用于创建复杂效果的一种机制。


如果要应用所创建的滤镜效果，只需要为 SVG 图形元素设置 filter 属性即可。


```html
<feGaussianBlur in="SourceAlpha" stdDeviation="4" result="blur" />
```

"SourceAlpha" 值，即原图像的 alpha 通道，并设置了模糊度为 4 以及把 result 保存在了一个名为 "blur" 的临时缓冲区中。


```html
<feOffset in="blur" dx="4" dy="4" result="offsetBlur" />

```

设置 in 的值为 "blur"，即我们前面保存 result 的那个临时缓冲区。
然后设置相对坐标，向右偏移 4，向下偏移 4。
最后把结果 result 保存到名为 "offsetBlur" 的缓冲区中。步骤 1、2 其实是创建图形阴影的两个图元。

```html
<feSpecularLighting
  in="offsetBlur"
  surfaceScale="5"
  specularConstant=".75"
  specularExponent="20"
  lighting-color="#bbbbbb"
  result="specOut">
  <fePointLight x="-5000" y="-10000" z="20000" />
</feSpecularLighting>

```

输入源 in 为 "offsetBlur"，将会生成一个光照效果，并将结果 result 保存在 "specOut" 中。

```html
<feComposite in="specOut" in2="SourceAlpha" operator="in" result="specOut" />

```

输入源为 "specOut"，第二个输入源（in2）为 "SourceAlpha"，将 "specOut" 的结果效果遮盖掉，以致于最后的结果不会大于 "SourceAlpha"（源图像），最后覆盖输出结果 result 为 "specOut"。


```html
<feComposite in="SourceGraphic" in2="specOut"
             operator="arithmetic"
             k1="0" k2="1" k3="1" k4="0"
             result="litPaint"/>
```

设置 in 为 "SourceGraphic" 和 "specOut"，即在 "SourceGraphic" 之上添加 "specOut" 的效果，复合模式为 "arithmetic"，然后保存结果为 "litPaint"。


```html

<feMerge>
  <feMergeNode in="offsetBlur"/>
  <feMergeNode in="litPaint"/>
</feMerge>
```
最后，<feMerge> 元素合并了阴影效果 "offsetBlur" 和源图像的光照效果 "litPaint"。