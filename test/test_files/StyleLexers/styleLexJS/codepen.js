function calcTickIncr(u, min, max) {
  let space = u.axes[0].space();
  let width = parseInt(u.ctx.canvas.style.width);
  let range = max - min;
  let tickQty = width / space;
  let tickIncr = range / tickQty;
  return tickIncr;
}

function mockFetch(minX, maxX, step) {
  console.log("Fetching data for range:", [minX, maxX]);

  return new Promise(res => {
    res(genData(minX, maxX, step));
  });
}

// how the "full" range's min/max is determined is going to be app-dependant; it's simply hard-coded here.
let allMin = 1546300800;    // Jan 1 2019 UTC
let allMax = 1546300801;

function resetZoomOnDblCick(plot) {
  plot.ctx.canvas.ondblclick = e => {
    zoom(timePlot, numPlot, allMin, allMax);
  };
}

function zoom(timePlot, numPlot, min, max) {
  // whether timePlot or numPlot is passed here doesn't matter as long as they use the same units
  // in this case we're always passing secs and `space` and `width` are the same in both
  let tickIncr = calcTickIncr(timePlot, min, max);

  // mockFetch must return either ms resolution timestamps or ns resolution small ints

  let dataMin, dataMax, dataStep, targPlot, othrPlot;

  if (tickIncr < 0.001) {
    targPlot = numPlot;
    othrPlot = timePlot;

    // get decimal parts of timestamps
    dataMin = +(min % 1).toFixed(12);
    dataMax = +(max % 1).toFixed(12);
    dataStep = 0.0001;
  }
  else {
    targPlot = timePlot;
    othrPlot = numPlot;

    dataMin = min;
    dataMax = max;
    dataStep = 0.001;
  }

  mockFetch(dataMin, dataMax, dataStep).then(data => {
    targPlot.setData(data);

    // reset selections
    targPlot.setSelect({width: 0}, false);
    othrPlot.setSelect({width: 0}, false);

    // show/hide plots
    targPlot.root.classList.add("active");
    othrPlot.root.classList.remove("active");
  });
}

function onSetSelect(u) {
  let min = u.posToVal(u.select.left, 'x');
  let max = u.posToVal(u.select.left + u.select.width, 'x');

  zoom(timePlot, numPlot, min, max);
}

const width = 1600;
const height = 400;
const tzDate = ts => uPlot.tzDate(new Date(ts * 1e3), 'Etc/UTC');
const cursor = {
  drag: {
    setScale: false,
    x: true,
    y: false,
  }
};

const timeOpts = {
  title: "Temporal",
  class: "active",
  width,
  height,
  tzDate,
  cursor,
  series: [
    {},
    {
      stroke: "red",
    }
  ],
  hooks: {
    init: [
      resetZoomOnDblCick,
    ],
    setSelect: [
      onSetSelect
    ]
  }
};

const numOpts = {
  title: "Numeric",
  width,
  height,
  tzDate,
  cursor,
  scales: {
    x: {
      time: false,
    }
  },
  series: [
    {},
    {
      stroke: "blue",
    }
  ],
  hooks: {
    init: [
      resetZoomOnDblCick,
    ],
    setSelect: [
      onSetSelect
    ]
  }
};

function genData(min, max, step) {
  let times = [];

  let cur = min, i = 0;

  do {
    times.push(cur);
    cur = +(min + step * i).toFixed(12);
    i++;
  } while (cur < max);

  let values = Array(times.length);

  for (let i = 0; i < times.length; i++)
    values[i] = i % 10 == 0 ? 1 : 0.5;

  return [
    times,
    values,
  ];
}

let timePlot = new uPlot.Line(timeOpts, genData(allMin, allMax, 0.001), document.body);
let numPlot = new uPlot.Line(numOpts, genData(0, 1, 0.1), document.body);    // initial fake data for plot
