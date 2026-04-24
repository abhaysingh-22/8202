// Static database representing 8 technological products
const products = [
    { id: 1, name: 'Over-Ear Headphones', category: 'Electronics', price: 299.99, image: 'https://images.unsplash.com/photo-1505740420928-5e560c06d30e?w=500&q=80' },
    { id: 2, name: 'Mechanical Keyboard', category: 'Peripherals', price: 129.50, image: 'https://images.unsplash.com/photo-1595225476474-87563907a212?w=500&q=80' },
    { id: 3, name: 'USB-C Docking Hub', category: 'Accessories', price: 45.00, image: 'https://images.unsplash.com/photo-1583863788434-e58a36330cf0?w=500&q=80' },
    { id: 4, name: 'Ergonomic Mouse', category: 'Peripherals', price: 79.99, image: 'https://images.unsplash.com/photo-1527864550417-7fd91fc51a46?w=500&q=80' },
    { id: 5, name: '4K Desktop Monitor', category: 'Electronics', price: 349.00, image: 'https://images.unsplash.com/photo-1527443224154-c4a3942d3acf?w=500&q=80' },
    { id: 6, name: 'Aluminum Laptop Stand', category: 'Accessories', price: 35.50, image: 'https://images.unsplash.com/photo-1611186871348-b1ce696e52c9?w=500&q=80' },
    { id: 7, name: '1080p Streaming Webcam', category: 'Electronics', price: 89.00, image: 'https://encrypted-tbn2.gstatic.com/shopping?q=tbn:ANd9GcTncRE9VVzFryLHiKPXInDhj-xws5Rlodw1QYNSWhKbbApPcX1ZT-sAvkYS-tzLS1eM-oPReaZmLEMqpSx1tWPevq_p1smQabe-PZzxQwXKSHrgwSwKwPvAoQ' },
    { id: 8, name: 'Extended Desk Mat', category: 'Accessories', price: 19.99, image: 'https://encrypted-tbn1.gstatic.com/shopping?q=tbn:ANd9GcSQiubAQbW1QFQnxPM2L4XNHBxdTunvHa1vFyYAxhgeXDmZkuGOD55NTCyJHPt5CFUhrKX-lzrJwKkUZjSCI7X5JMCocNbXSQ' }
];

// DOM Elements
const grid = document.getElementById('product-grid');
const searchInput = document.getElementById('searchInput');
const categoryFilter = document.getElementById('categoryFilter');

/**
 * Renders the products to the grid. Uses flexbox trickery dynamically via CSS 
 * to handle any resize nicely inside a single view frame.
 */
function renderProducts(items) {
    // Clear out grid
    grid.innerHTML = '';
    
    // Fallback if search/filter leaves 0 items
    if (items.length === 0) {
        grid.innerHTML = '<div class="no-results">No products found matching your criteria.</div>';
        return;
    }

    // Build the grid cards dynamically
    items.forEach(product => {
        const card = document.createElement('div');
        card.className = 'product-card';
        card.innerHTML = `
            <div class="image-placeholder" style="background-image: url('${product.image}'); background-color: #eee;"></div>
            <div class="info">
                <h3>${product.name}</h3>
                <p class="category">${product.category}</p>
                <p class="price">$${product.price.toFixed(2)}</p>
            </div>
        `;
        grid.appendChild(card);
    });
}

/**
 * Filter function triggered on input text or dropdown change
 */
function filterProducts() {
    const searchTerm = searchInput.value.toLowerCase().trim();
    const selectedCategory = categoryFilter.value;

    const filtered = products.filter(product => {
        const matchesSearch = product.name.toLowerCase().includes(searchTerm);
        const matchesCategory = selectedCategory === 'All' || product.category === selectedCategory;
        
        // Product must pass both the category check and the search text check
        return matchesSearch && matchesCategory;
    });

    renderProducts(filtered);
}

// Attach Event Listeners
searchInput.addEventListener('input', filterProducts);
categoryFilter.addEventListener('change', filterProducts);

// Initial bootstrap rendering on page load
renderProducts(products);