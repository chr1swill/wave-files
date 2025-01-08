(function(){
  if (process.argv.length != 3) {
    console.error("please provide a single command line arg to program\n");
    return 1;
  }

  const s = process.argv[2];
  console.log("arg: ", s);

  let output = [];
  
  for (let i = 0; i < s.length; i++) {
    const cc = s.charCodeAt(i);
    output.push(cc.toString(16).toUpperCase());
  }

  console.log("0x" + output.reverse().join(""));
})();
