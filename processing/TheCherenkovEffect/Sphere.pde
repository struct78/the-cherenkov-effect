import java.util.Collections;

class Sphere {
  int numPoints;
  int radius;
  ArrayList<PVector> points;
  float theta;
  
  Sphere(int numPoints, int radius) {
    this.numPoints = numPoints;
    this.radius = radius;
  }
  
  void seed() {
    this.points = new ArrayList<PVector>();
    for (int x = 0 ; x < this.numPoints ; x++) {
      float lon = map(x, 0, this.numPoints, -PI, PI);
      for (int y = 0 ; y < this.numPoints ; y++) {
        float lat = map(y, 0, this.numPoints, -HALF_PI, HALF_PI);
        float r = this.radius + (noise(x, y, theta) * 2);
        float x0 = r * sin(lon) * cos(lat) + noise(x, y, theta);
        float y0 = r * sin(lon) * sin(lat) + noise(x, y, theta);
        float z0 = r * cos(lon) + noise(x, y, theta);
        PVector vec = new PVector(x0, y0, z0);
        this.points.add(vec);
      }
      theta += 0.0024;
    }
    theta = 0;
  }
  
  void draw() {
    background(255);
    noFill();
    stroke(0);
    strokeWeight(1);
    translate(width/2, height/2);
    rotateY(theta);
    rotateX(theta);
    rotateZ(theta / 4);
    
    beginShape();
    int x = 0;
    for (PVector p : points) {
      PVector vec = p.copy();
      //vec.mult(map(noise(theta, vec.x, vec.y), 0, 1, 0.5, 1.5));
      vec.mult(1.1);
      //point(vec.x, vec.y, vec.z);
      vertex(vec.x, vec.y, vec.z);
      x++;
    }
    theta += 0.015;
    endShape();
  }
}
